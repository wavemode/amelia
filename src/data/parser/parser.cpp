#include "parser.hpp"
#include "prelude.hpp"

#include "data/lexer/lexer_result.hpp"
#include "data/lexer/token_formatter.hpp"
#include "data/parser/parser_error.hpp"
#include "data/parser/parser_result.hpp"

namespace amelia {

namespace {

bool is_identifier(TokenType type) {
  return type == TokenType::IDENTIFIER || type == TokenType::IDENTIFIER_NO_W ||
         type == TokenType::QUOTED_IDENTIFIER || type == TokenType::QUOTED_IDENTIFIER_NO_W ||
         type == TokenType::KEYWORD_THIS_TYPE;
}

bool is_identifier_no_w(TokenType type) {
  return type == TokenType::IDENTIFIER_NO_W || type == TokenType::QUOTED_IDENTIFIER_NO_W;
}

struct TokenWithId {
  TokenId id;
  TokenType type;
  Location loc;
};

class ParserState {
public:
  ParserState(ParserResult &output, const LexerResult &input)
      : m_output(output), m_input(input), m_token_index(0), m_token_formatter(input), m_imports(),
        m_submodules() {}

  NodeId parse_module() {
    List<NodeId> decls;
    m_imports.clear();
    m_submodules.clear();
    auto start_token = peek();
    parse_top_level_decls(decls, TokenType::END_OF_FILE);
    return m_output.add_node(
        start_token.id, m_token_index, ModuleNode{move(decls), move(m_imports), move(m_submodules)}
    );
  }

  void parse_top_level_decls(List<NodeId> &decls, TokenType terminator) {
    while (peek().type != terminator) {
      auto decl = try_parse_top_level_decl();
      if (!decl.has_value()) {
        throw_parser_error_at_current_location("Expected declaration");
      }
      decls.push_back(decl.value());

      if (decls.size() > 1) {
        enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
      }
    }
  }

  void parse_statements(List<NodeId> &stmts, TokenType terminator) {
    while (peek().type != terminator) {
      stmts.push_back(parse_statement());
      if (stmts.size() > 1) {
        enforce_statement_separator(stmts[stmts.size() - 2], stmts[stmts.size() - 1]);
      }
    }
  }

  List<NodeId> parse_introductory_decls() {
    List<NodeId> decls;
    while (true) {
      auto decl = try_parse_decl();
      if (!decl.has_value()) {
        break;
      }
      decls.push_back(decl.value());
      if (decls.size() > 1) {
        enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
      }
    }
    return decls;
  }

  void enforce_statement_separator(NodeId left, NodeId right) {
    const Node &stmt = m_output.get_node(left);
    const Node &next_stmt = m_output.get_node(right);
    if (stmt.type() != NodeType::EmptyStmtNode && next_stmt.type() != NodeType::EmptyStmtNode) {
      auto stmt_start_token = m_input.get_token(stmt.start_token());
      auto next_stmt_start_token = m_input.get_token(next_stmt.start_token());
      if (stmt_start_token.location.line == next_stmt_start_token.location.line) {
        throw_parser_error_at_location(
            next_stmt_start_token.location,
            "Multiple statements on the same line must be separated by a semicolon"
        );
      }
    }
  }

  void enforce_separator_after_introductory_decls(const List<NodeId> &decls, NodeId next_stmt_id) {
    if (decls.size() > 0) {
      enforce_statement_separator(decls[decls.size() - 1], next_stmt_id);
    }
  }

  Option<NodeId> try_parse_top_level_decl() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_LOCAL:
      return parse_top_level_visibility_decl();
    case TokenType::KEYWORD_PUBLIC:
      if (peek(1).type == TokenType::KEYWORD_IMPORT) {
        return parse_top_level_visibility_decl();
      }
      break;
    case TokenType::KEYWORD_IMPORT:
      return parse_import_decl();
    case TokenType::KEYWORD_EXTERN:
      return parse_extern_top_level_decl();
    default:
      break;
    }
    return try_parse_decl();
  }

  NodeId parse_extern_top_level_decl() {
    auto extern_token = next();
    auto decl = expect_top_level_decl("Expected top-level decl after 'extern' keyword");
    return m_output.add_node(extern_token.id, m_token_index, ExternDeclNode{decl});
  }

  NodeId expect_top_level_decl(Text error_message) {
    auto decl = try_parse_top_level_decl();
    if (!decl.has_value()) {
      throw_parser_error_at_current_location(String(error_message));
    }
    return decl.value();
  }

  NodeId parse_import_decl() {
    auto import_token = next();
    auto next_token = peek();
    if (!is_identifier(next_token.type)) {
      throw_parser_error_at_current_location(
          "Expected identifier after 'import' keyword in import declaration"
      );
    }
    auto path = parse_scoped_name();
    auto items = try_parse_import_items();
    auto alias = try_parse_import_alias();
    auto result = m_output.add_node(
        import_token.id, m_token_index, ImportDeclNode{path, move(items), alias}
    );
    m_imports.push_back(result);
    return result;
  }

  Option<NodeId> try_parse_import_alias() {
    if (peek().type == TokenType::KEYWORD_AS) {
      ++m_token_index; // consume the 'as' token
      if (!is_identifier(peek().type)) {
        throw_parser_error_at_current_location(
            "Expected identifier after 'as' keyword in import alias"
        );
      }
      return parse_identifier();
    }
    return None();
  }

  Option<List<NodeId>> try_parse_import_items() {
    auto start_token = peek();
    Option<List<NodeId>> result;
    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the '(' token
      List<NodeId> items;
      while (peek().type != TokenType::RIGHT_PAREN) {
        items.push_back(parse_import_item());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        }
      }
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' to end import item list");
      result = move(items);
    }
    return result;
  }

  NodeId parse_import_item() {
    auto item_token = peek();
    if (item_token.type == TokenType::ELLIPSIS) {
      ++m_token_index; // consume the '...' token
      return m_output.add_node(item_token.id, m_token_index, ImportItemWildcardNode{});
    } else {
      auto name = expect_identifier("Expected identifier or '...' in import item list");
      auto sub_items = try_parse_import_items();
      auto alias = try_parse_import_alias();
      return m_output.add_node(
          item_token.id, m_token_index, ImportItemNode{name, move(sub_items), alias}
      );
    }
  }

  Option<NodeId> try_parse_decl() {
    auto token = peek();
    switch (token.type) {
    case TokenType::KEYWORD_MODULE:
      return parse_module_decl();
    case TokenType::KEYWORD_LET:
      return parse_let_decl();
    case TokenType::KEYWORD_CONST:
      return parse_const_decl();
    case TokenType::KEYWORD_FUN: {
      if (is_identifier(peek(1).type)) {
        return parse_function_decl();
      }
    } break;
    case TokenType::KEYWORD_CLASS:
      return parse_class_decl();
    case TokenType::KEYWORD_TYPE:
      return parse_type_decl();
    case TokenType::KEYWORD_OPERATOR:
      return parse_operator_function_decl();
    case TokenType::SEMICOLON:
      return parse_empty_statement();
    case TokenType::KEYWORD_CONCEPT:
      return parse_concept_decl();
    case TokenType::KEYWORD_OPEN:
      return parse_open_decl();
    case TokenType::KEYWORD_ASYNC:
      return parse_async_decl();
    case TokenType::KEYWORD_RECORD:
      return parse_record_decl();
    case TokenType::KEYWORD_SEALED:
      return parse_sealed_decl();
    case TokenType::KEYWORD_UNION:
      return parse_union_decl();
    case TokenType::KEYWORD_ENUM:
      return parse_enum_decl();
    case TokenType::KEYWORD_INLINE:
      return parse_inline_decl();
    case TokenType::AT:
      return parse_annotated_decl();
    default:
      break;
    }
    return None();
  }

  NodeId parse_inline_decl() {
    auto inline_token = next();
    auto decl = expect_decl("Expected declaration after 'inline' keyword");
    return m_output.add_node(inline_token.id, m_token_index, InlineDeclNode{decl});
  }

  NodeId parse_sealed_decl() {
    auto sealed_token = next();
    auto decl = expect_decl("Expected declaration after 'sealed' keyword");
    return m_output.add_node(sealed_token.id, m_token_index, SealedDeclNode{decl});
  }

  NodeId parse_annotated_decl() {
    auto at_token = next();
    auto name = parse_annotation_name();
    auto args = try_parse_annotation_arguments();
    auto stmt = expect_decl("Expected declaration after annotation");
    return m_output.add_node(at_token.id, m_token_index, AnnotationNode{name, args, stmt});
  }

  NodeId parse_enum_decl() {
    auto enum_token = next();
    auto name = expect_identifier("Expected enum name after 'enum' keyword");
    Option<NodeId> repr_type;
    auto next_token = peek();
    if (next_token.type == TokenType::LEFT_PAREN || next_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the '(' token
      repr_type = parse_type_expr();
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after enum representation type");
    }
    auto base_types = try_parse_base_type_list();
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to begin enum declaration");
    List<NodeId> variants;
    while (peek().type != TokenType::RIGHT_BRACE) {
      variants.push_back(parse_enum_variant());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
      }
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end enum declaration");
    return m_output.add_node(
        enum_token.id, m_token_index, EnumDeclNode{name, repr_type, base_types, move(variants)}
    );
  }

  NodeId parse_enum_variant() {
    auto start_token = peek();
    auto name = expect_identifier("Expected enum variant name");
    Option<NodeId> value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      value = parse_expr();
    }
    return m_output.add_node(start_token.id, m_token_index, EnumVariantNode{name, value});
  }

  NodeId parse_record_decl() {
    auto record_token = next();
    auto decl = expect_decl("Expected declaration after 'record' keyword");
    return m_output.add_node(record_token.id, m_token_index, RecordDeclNode{decl});
  }

  NodeId parse_union_decl() {
    auto union_token = next();
    auto name = parse_scoped_name();
    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    Option<NodeId> body = try_parse_class_body();
    return m_output.add_node(
        union_token.id, m_token_index, UnionDeclNode{name, generic_parameter_list, body}
    );
  }

  NodeId parse_async_decl() {
    auto async_token = next();
    auto decl = expect_decl("Expected declaration after 'async' keyword");
    return m_output.add_node(async_token.id, m_token_index, AsyncDeclNode{decl});
  }

  NodeId parse_open_decl() {
    auto open_token = next();
    auto decl = expect_decl("Expected declaration after 'open' keyword");
    return m_output.add_node(open_token.id, m_token_index, OpenDeclNode{decl});
  }

  NodeId parse_module_decl() {
    auto module_token = next();
    auto name = expect_identifier("Expected module name after 'module' keyword");
    if (peek().type == TokenType::LEFT_BRACE) {
      ++m_token_index; // consume the '{' token
      List<NodeId> decls;
      List<NodeId> old_submodules = move(m_submodules);
      m_submodules.clear();
      parse_top_level_decls(decls, TokenType::RIGHT_BRACE);
      read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end module declaration");
      auto result = m_output.add_node(
          module_token.id, m_token_index, ModuleDeclNode{name, move(decls), move(m_submodules)}
      );
      m_submodules = move(old_submodules);
      m_submodules.push_back(result);
      return result;
    } else {
      return m_output.add_node(
          module_token.id, m_token_index, ModuleDeclNode{name, None(), None()}
      );
    }
  }

  NodeId expect_decl(Text error_message) {
    auto decl = try_parse_decl();
    if (!decl.has_value()) {
      throw_parser_error_at_current_location(String(error_message));
    }
    return decl.value();
  }

  NodeId expect_local_decl(Text error_message) {
    auto decl = try_parse_decl();
    if (!decl.has_value()) {
      throw_parser_error_at_current_location(String(error_message));
    }
    return decl.value();
  }

  NodeId parse_statement() {
    auto token = peek();
    switch (token.type) {
    case TokenType::DOUBLE_PLUS:
    case TokenType::DOUBLE_PLUS_NO_W:
      return parse_pre_increment_statement();
    case TokenType::DOUBLE_MINUS:
    case TokenType::DOUBLE_MINUS_NO_W:
      return parse_pre_decrement_statement();
    case TokenType::LEFT_BRACE:
      return parse_block_statement();
    case TokenType::KEYWORD_IF:
      return parse_if_statement();
    case TokenType::KEYWORD_SWITCH:
      return parse_switch_statement();
    case TokenType::KEYWORD_TRY:
      return parse_try_statement();
    case TokenType::KEYWORD_THROW:
      return parse_throw_statement();
    case TokenType::KEYWORD_FOR:
      return parse_for_in_statement();
    case TokenType::KEYWORD_WHILE:
      return parse_while_statement();
    case TokenType::KEYWORD_LABEL:
      return parse_label_statement();
    case TokenType::KEYWORD_GOTO:
      return parse_goto_statement();
    case TokenType::KEYWORD_CONTINUE:
      return parse_continue_statement();
    case TokenType::KEYWORD_RETURN:
      return parse_return_statement();
    case TokenType::KEYWORD_BREAK:
      return parse_break_statement();
    case TokenType::AT:
      return parse_annotated_statement();
    default:
      break;
    }
    auto decl = try_parse_decl();
    if (decl.has_value()) {
      return decl.value();
    }
    return parse_expr_statement();
  }

  NodeId parse_annotated_statement() {
    auto at_token = next();
    auto name = parse_annotation_name();
    auto args = try_parse_annotation_arguments();
    auto stmt = parse_statement();
    return m_output.add_node(at_token.id, m_token_index, AnnotationNode{name, args, stmt});
  }

  NodeId parse_annotation_name() {
    auto token = peek();
    if (!is_identifier_no_w(token.type)) {
      throw_parser_error_at_current_location(
          "Expected identifier immediately after '@' in annotation name"
      );
    }
    return parse_identifier();
  }

  Option<List<NodeId>> try_parse_annotation_arguments() {
    if (peek().type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the '(' token
      List<NodeId> args_list;
      while (peek().type != TokenType::RIGHT_PAREN) {
        args_list.push_back(parse_function_call_argument());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the comma
        }
      }
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after annotation arguments");
      return Some(move(args_list));
    }
    return None();
  }

  NodeId parse_break_statement() {
    auto break_token = next();
    return m_output.add_node(break_token.id, m_token_index, BreakStmtNode{});
  }

  NodeId parse_operator_function_decl() {
    auto operator_token = peek();
    auto operator_ident = parse_operator_ident();
    auto signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(
        operator_token.id, m_token_index, OperatorFunctionDeclNode{operator_ident, signature, body}
    );
  }

  NodeId parse_top_level_visibility_decl() {
    auto visibility_token = next();
    DeclVisibility visibility;
    switch (visibility_token.type) {
    case TokenType::KEYWORD_PUBLIC:
      visibility = DeclVisibility::Public;
      break;
    case TokenType::KEYWORD_PRIVATE:
      visibility = DeclVisibility::Private;
      break;
    case TokenType::KEYWORD_PROTECTED:
      visibility = DeclVisibility::Protected;
      break;
    case TokenType::KEYWORD_LOCAL:
      visibility = DeclVisibility::Local;
      break;
    default:
      throw_parser_error_at_current_location("Invalid visibility keyword");
    }

    Option<NodeId> scope;
    auto next_token = peek();
    if (next_token.type == TokenType::LEFT_PAREN || next_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the '(' token
      if (!is_identifier(peek().type)) {
        throw_parser_error_at_current_location(
            "Expected identifier after '(' in visibility declaration"
        );
      }
      scope = parse_scoped_name();
      read_token_type(
          TokenType::RIGHT_PAREN, "Expected ')' after scope name in visibility declaration"
      );
    }

    NodeId decl = expect_top_level_decl("Expected top-level decl after visibility modifier");
    return m_output.add_node(
        visibility_token.id, m_token_index, VisibilityNode{visibility, scope, decl}
    );
  }

  NodeId parse_concept_decl() {
    auto concept_token = next();
    auto name = expect_identifier("Expected concept name after 'concept' keyword");
    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    Option<NodeId> base_concept_list = try_parse_base_type_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    Option<NodeId> body = try_parse_class_body();
    return m_output.add_node(
        concept_token.id,
        m_token_index,
        ConceptDeclNode{
            name, generic_parameter_list, base_concept_list, implicit_parameter_list, body
        }
    );
  }

  NodeId parse_class_decl() {
    auto class_token = next();
    if (!is_identifier(peek().type)) {
      throw_parser_error_at_current_location(
          "Expected identifier after 'class' keyword in class declaration"
      );
    }
    auto name = parse_scoped_name();
    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    Option<NodeId> header_decls = try_parse_class_header_decls();
    Option<NodeId> base_class_list = try_parse_base_type_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    Option<NodeId> body = try_parse_class_body();
    return m_output.add_node(
        class_token.id,
        m_token_index,
        ClassDeclNode{
            name,
            generic_parameter_list,
            base_class_list,
            header_decls,
            implicit_parameter_list,
            body
        }
    );
  }

  Option<NodeId> try_parse_class_header_decls() {
    Option<NodeId> result;
    auto start_token = peek();
    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the '(' token
      List<NodeId> decls;
      while (peek().type != TokenType::RIGHT_PAREN) {
        decls.push_back(parse_class_header_decl());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        } else {
          break;
        }
      }
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' to end class header decls");
      result = m_output.add_node(start_token.id, m_token_index, ClassHeaderDeclsNode{move(decls)});
    }
    return result;
  }

  NodeId parse_class_header_decl() {
    auto next_token = peek();
    switch (next_token.type) {
    case TokenType::KEYWORD_CONST:
      return parse_class_header_const_decl();
    default:
      return parse_class_header_field_decl();
    }
  }

  NodeId parse_class_header_field_decl() {
    auto start_token = peek();
    Option<NodeId> name;

    if (is_identifier(start_token.type) && peek(1).type == TokenType::COLON) {
      name = parse_identifier();
      ++m_token_index; // consume the ':' token
    }

    NodeId type = parse_expr();

    Option<NodeId> default_value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      default_value = parse_expr();
    }

    return m_output.add_node(
        start_token.id, m_token_index, ClassHeaderFieldDeclNode{name, type, default_value}
    );
  }

  NodeId parse_class_header_const_decl() {
    auto const_token = next();
    auto decl = parse_class_header_decl();
    return m_output.add_node(const_token.id, m_token_index, ClassHeaderConstDeclNode{decl});
  }

  Option<NodeId> try_parse_class_body() {
    Option<NodeId> result;
    if (peek().type == TokenType::LEFT_BRACE) {
      ++m_token_index; // consume the '{' token
      List<NodeId> decls;
      while (peek().type != TokenType::RIGHT_BRACE) {
        decls.push_back(parse_class_body_decl());
        if (decls.size() > 1) {
          enforce_statement_separator(decls[decls.size() - 2], decls[decls.size() - 1]);
        }
      }
      read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end class body");
      result = m_output.add_node(peek(-1).id, m_token_index, ClassBodyNode{move(decls)});
    }
    return result;
  }

  Option<NodeId> try_parse_base_type_list() {
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      List<NodeId> base_types;
      base_types.push_back(expect_identifier("Expected base type name after ':'"));
      while (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
        base_types.push_back(parse_type_expr());
      }
      return Some(m_output.add_node(peek(-1).id, m_token_index, BaseTypeListNode{move(base_types)})
      );
    }
    return None();
  }

  NodeId parse_class_body_decl() {
    auto next_token = peek();
    switch (next_token.type) {
    case TokenType::KEYWORD_STATIC:
      return parse_class_static_decl();
    case TokenType::KEYWORD_CONST:
      return parse_class_const_decl();
    case TokenType::KEYWORD_COPY:
      return parse_class_copy_decl();
    case TokenType::KEYWORD_MOVE: {
      return parse_class_move_decl();
    }
    case TokenType::IDENTIFIER:
    case TokenType::IDENTIFIER_NO_W:
    case TokenType::QUOTED_IDENTIFIER:
    case TokenType::QUOTED_IDENTIFIER_NO_W:
    case TokenType::KEYWORD_THIS_TYPE: {
      auto following_token = peek(1);
      if (following_token.type == TokenType::LEFT_PAREN ||
          following_token.type == TokenType::LEFT_PAREN_NO_W ||
          following_token.type == TokenType::LEFT_BRACKET ||
          following_token.type == TokenType::LEFT_BRACKET_NO_W) {
        return parse_constructor_decl();
      }
      return parse_class_field();
    } break;
    case TokenType::KEYWORD_PUBLIC:
    case TokenType::KEYWORD_PRIVATE:
    case TokenType::KEYWORD_PROTECTED:
    case TokenType::KEYWORD_LOCAL:
      return parse_class_body_visibility_decl();
    case TokenType::KEYWORD_IMPLICIT:
      return parse_class_body_implicit_decl();
    case TokenType::KEYWORD_DEFAULT:
      return parse_class_body_default_decl();
    case TokenType::KEYWORD_OPEN:
      return parse_class_body_open_decl();
    case TokenType::KEYWORD_OVERRIDE:
      return parse_class_body_override_decl();
    case TokenType::TILDE:
      return parse_destructor_decl();
    case TokenType::AT:
      return parse_class_body_annotated_decl();
    case TokenType::KEYWORD_MUT:
      return parse_mut_class_body_decl();
    default:
      break;
    }
    return expect_local_decl("Expected declaration in class body");
  }

  NodeId parse_mut_class_body_decl() {
    auto mut_token = next();
    auto decl = parse_class_body_decl();
    return m_output.add_node(mut_token.id, m_token_index, MutDeclNode{decl});
  }

  NodeId parse_class_body_annotated_decl() {
    auto at_token = next();
    auto name = parse_annotation_name();
    auto args = try_parse_annotation_arguments();
    auto stmt = parse_class_body_decl();
    return m_output.add_node(at_token.id, m_token_index, AnnotationNode{name, args, stmt});
  }

  NodeId parse_destructor_decl() {
    ++m_token_index; // consume the '~' token
    auto name_token = peek();
    NodeId name = expect_identifier("Expected destructor name");
    NodeId signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(
        name_token.id, m_token_index, ClassDestructorNode{name, signature, body}
    );
  }

  NodeId parse_class_body_open_decl() {
    auto open_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(open_token.id, m_token_index, OpenDeclNode{decl});
  }

  NodeId parse_class_body_override_decl() {
    auto override_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(override_token.id, m_token_index, OverrideDeclNode{decl});
  }

  NodeId parse_class_body_default_decl() {
    auto default_token = next();
    auto decl = parse_class_body_decl();
    return m_output.add_node(default_token.id, m_token_index, DefaultDeclNode{decl});
  }

  NodeId parse_class_body_implicit_decl() {
    auto implicit_token = next();
    auto decl = parse_class_body_decl();
    return m_output.add_node(implicit_token.id, m_token_index, ImplicitDeclNode{decl});
  }

  NodeId parse_class_body_visibility_decl() {
    auto visibility_token = next();
    DeclVisibility visibility;
    switch (visibility_token.type) {
    case TokenType::KEYWORD_PUBLIC:
      visibility = DeclVisibility::Public;
      break;
    case TokenType::KEYWORD_PRIVATE:
      visibility = DeclVisibility::Private;
      break;
    case TokenType::KEYWORD_PROTECTED:
      visibility = DeclVisibility::Protected;
      break;
    case TokenType::KEYWORD_LOCAL:
      visibility = DeclVisibility::Local;
      break;
    default:
      throw RuntimeError("Invalid visibility modifier");
    }
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(
        visibility_token.id, m_token_index, VisibilityNode{visibility, None(), decl}
    );
  }

  NodeId parse_constructor_decl() {
    auto name_token = peek();
    NodeId name;
    if (name_token.type == TokenType::KEYWORD_COPY) {
      name = m_output.add_node(name_token.id, m_token_index, CopyCtorNameNode{});
      ++m_token_index; // consume the 'copy' keyword
    } else if (name_token.type == TokenType::KEYWORD_MOVE) {
      name = m_output.add_node(name_token.id, m_token_index, MoveCtorNameNode{});
      ++m_token_index; // consume the 'move' keyword
    } else {
      name = expect_identifier("Expected constructor name");
    }
    NodeId signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(
        name_token.id, m_token_index, ClassConstructorNode{name, signature, body}
    );
  }

  NodeId parse_class_static_decl() {
    auto static_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(static_token.id, m_token_index, ClassStaticDeclNode{decl});
  }

  NodeId parse_class_const_decl() {
    auto const_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(const_token.id, m_token_index, ClassConstDeclNode{decl});
  }

  NodeId parse_class_copy_decl() {
    auto copy_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(copy_token.id, m_token_index, ClassCopyDeclNode{decl});
  }

  NodeId parse_class_move_decl() {
    auto move_token = next();
    NodeId decl = parse_class_body_decl();
    return m_output.add_node(move_token.id, m_token_index, ClassMoveDeclNode{decl});
  }

  NodeId parse_class_field() {
    auto start_token = peek();
    auto name = expect_identifier("Expected field name in class declaration");
    Option<NodeId> type;
    Option<NodeId> initializer;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type = parse_type_expr();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      initializer = parse_expr();
    }
    return m_output.add_node(
        start_token.id, m_token_index, ClassFieldNode{name, type, initializer}
    );
  }

  Option<NodeId> try_parse_generic_parameter_list() {
    auto start_token = peek();
    if (start_token.type == TokenType::LEFT_BRACKET_NO_W ||
        start_token.type == TokenType::LEFT_BRACKET) {
      ++m_token_index; // consume the left bracket
      List<NodeId> parameters;
      do {
        parameters.push_back(parse_generic_parameter());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the comma
        }
      } while (peek().type != TokenType::RIGHT_BRACKET);
      read_token_type(TokenType::RIGHT_BRACKET, "Expected ']' to end generic parameter list");

      Option<List<NodeId>> additional_constraints;
      if (peek().type == TokenType::KEYWORD_WHEN) {
        ++m_token_index; // consume the 'when' keyword
        read_left_paren("Expected '(' after 'when' in generic parameter list");
        List<NodeId> constraints;
        do {
          constraints.push_back(parse_type_constraint());
          if (peek().type == TokenType::COMMA) {
            ++m_token_index; // consume the comma
          }
        } while (peek().type != TokenType::RIGHT_PAREN);
        read_token_type(
            TokenType::RIGHT_PAREN,
            "Expected ')' to end additional constraints in generic parameter list"
        );
        additional_constraints = move(constraints);
      }

      return Some(m_output.add_node(
          start_token.id,
          m_token_index,
          GenericParameterListNode{move(parameters), move(additional_constraints)}
      ));
    }
    return None();
  }

  NodeId parse_generic_parameter() {
    auto start_token = peek();
    bool is_const = false;
    if (start_token.type == TokenType::KEYWORD_CONST) {
      is_const = true;
      ++m_token_index; // consume the 'const' keyword
    }
    NodeId type = expect_identifier("Expected type name for generic parameter");
    Option<NodeId> constraint;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      constraint = parse_type_expr();
    }

    Option<NodeId> default_value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      default_value = parse_type_expr();
    }
    return m_output.add_node(
        start_token.id,
        m_token_index,
        GenericParameterNode{is_const, type, constraint, default_value}
    );
  }

  NodeId parse_type_constraint() {
    auto start_token = peek();
    NodeId lhs = parse_type_expr();
    Option<NodeId> rhs;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      rhs = parse_type_expr();
    }
    return m_output.add_node(start_token.id, m_token_index, TypeConstraintNode{lhs, rhs});
  }

  NodeId parse_try_statement() {
    auto try_token = next();
    NodeId try_block = parse_statement();
    List<NodeId> catch_clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      catch_clauses.push_back(parse_try_statement_catch_clause());
    }
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' token
      else_branch = parse_statement();
    }
    return m_output.add_node(
        try_token.id, m_token_index, TryStmtNode{try_block, move(catch_clauses), else_branch}
    );
  }

  NodeId parse_try_statement_catch_clause() {
    auto catch_token = next();
    read_left_paren("Expected '(' after 'catch'");
    Option<NodeId> var;
    auto next_token = peek();
    auto following_token = peek(1);
    if (is_identifier(next_token.type) && following_token.type == TokenType::COLON) {
      var = parse_identifier();
      ++m_token_index; // consume the ':' token
    }
    NodeId exc_type = parse_type_expr();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_statement();
    return m_output.add_node(catch_token.id, m_token_index, CatchClauseNode{exc_type, var, body});
  }

  NodeId parse_switch_statement() {
    auto switch_token = next();
    read_left_paren("Expected '(' after 'switch'");
    auto introductory_decls = parse_introductory_decls();
    NodeId expr = parse_expr();
    enforce_separator_after_introductory_decls(introductory_decls, expr);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch statement");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch statement body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_switch_statement_case_clause());
    }

    Option<NodeId> default_body;
    if (peek().type == TokenType::KEYWORD_DEFAULT) {
      ++m_token_index; // consume the 'default' keyword
      default_body = parse_statement();
    }

    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch statement body");
    return m_output.add_node(
        switch_token.id,
        m_token_index,
        SwitchStmtNode{move(introductory_decls), expr, move(clauses), default_body}
    );
  }

  NodeId parse_switch_statement_case_clause() {
    auto case_token = next();
    NodeId header = parse_case_clause_header();
    NodeId body = parse_statement();
    return m_output.add_node(case_token.id, m_token_index, CaseClauseNode{header, body});
  }

  NodeId parse_type_decl() {
    auto type_token = next();
    if (!is_identifier(peek().type)) {
      throw_parser_error_at_current_location("Expected type name after 'type' keyword");
    }
    auto name = parse_scoped_name();
    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    Option<NodeId> type_expr;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      type_expr = parse_type_expr();
    }
    return m_output.add_node(
        type_token.id, m_token_index, TypeDeclNode{name, generic_parameter_list, type_expr}
    );
  }

  NodeId parse_continue_statement() {
    auto continue_token = next();
    return m_output.add_node(continue_token.id, m_token_index, ContinueStmtNode{});
  }

  NodeId parse_return_statement() {
    auto return_token = next();
    Option<NodeId> expr;
    auto next_token = peek();
    if (next_token.loc.line == return_token.loc.line && next_token.type != TokenType::SEMICOLON &&
        next_token.type != TokenType::END_OF_FILE && next_token.type != TokenType::RIGHT_BRACE) {
      expr = parse_expr();
    }
    return m_output.add_node(return_token.id, m_token_index, ReturnStmtNode{expr});
  }

  NodeId parse_function_decl() {
    auto fun_token = next();

    if (!is_identifier(peek().type)) {
      throw_parser_error_at_current_location("Expected function name after 'fun' keyword");
    }
    auto name = parse_scoped_name();

    NodeId signature = parse_function_signature();
    Option<NodeId> body = try_parse_function_body();
    return m_output.add_node(fun_token.id, m_token_index, FunctionDeclNode{name, signature, body});
  }

  Option<NodeId> try_parse_function_body() {
    auto start_token = peek();
    if (start_token.type != TokenType::LEFT_BRACE && start_token.type != TokenType::ASSIGN) {
      return None();
    }
    ++m_token_index; // consume the '{' or '=' token

    Option<NodeId> expr;
    Option<List<NodeId>> stmts;
    bool is_default = false;
    bool is_deleted = false;

    if (start_token.type == TokenType::ASSIGN) {
      auto next_token = peek();
      if (next_token.type == TokenType::KEYWORD_DEFAULT) {
        is_default = true;
        ++m_token_index; // consume the 'default' keyword
      } else if (next_token.type == TokenType::KEYWORD_DELETE) {
        is_deleted = true;
        ++m_token_index; // consume the 'delete' keyword
      } else {
        expr = parse_expr();
      }
    } else {
      stmts = List<NodeId>();
      parse_statements(stmts.value(), TokenType::RIGHT_BRACE);
      ++m_token_index; // consume the '}' token
    }

    return m_output.add_node(
        start_token.id, m_token_index, FunctionBodyNode{expr, move(stmts), is_default, is_deleted}
    );
  }

  NodeId parse_function_signature() {
    auto start_token = peek();

    Option<NodeId> generic_parameter_list = try_parse_generic_parameter_list();
    List<NodeId> parameters = parse_function_parameter_list();
    Option<NodeId> implicit_parameter_list = try_parse_implicit_parameter_list();
    Option<NodeId> capture_list = try_parse_function_signature_capture_annotation_list();
    Option<NodeId> return_type = try_parse_function_return_type();

    return m_output.add_node(
        start_token.id,
        m_token_index,
        FunctionSignatureNode{
            generic_parameter_list,
            move(parameters),
            implicit_parameter_list,
            capture_list,
            return_type
        }
    );
  }

  List<NodeId> parse_function_parameter_list() {
    read_left_paren("Expected '(' at the beginning of function parameter list");
    List<NodeId> parameters;
    while (peek().type != TokenType::RIGHT_PAREN) {
      parameters.push_back(parse_function_parameter());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the ',' token
      }
    }
    ++m_token_index; // consume the ')' token
    return parameters;
  }

  NodeId parse_function_parameter() {
    auto start_token = peek();
    bool variadic = false;
    if (peek().type == TokenType::ELLIPSIS) {
      variadic = true;
      ++m_token_index; // consume the '...' token
    }
    auto name = parse_expr();
    Option<NodeId> type;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type = parse_type_expr();
    }

    Option<NodeId> default_value;
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      default_value = parse_expr();
    }

    return m_output.add_node(
        start_token.id, m_token_index, FunctionParameterNode{variadic, name, type, default_value}
    );
  }

  Option<NodeId> try_parse_implicit_parameter_list() {
    auto next_token = peek();
    if (next_token.type == TokenType::KEYWORD_WITH) {
      ++m_token_index; // consume the 'with' keyword
      read_left_paren("Expected '(' after 'with' keyword in function signature");
      List<NodeId> implicit_parameters;
      while (peek().type != TokenType::RIGHT_PAREN) {
        implicit_parameters.push_back(parse_function_parameter());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        }
      }
      ++m_token_index; // consume the ')' token
      return Some(m_output.add_node(
          next_token.id, m_token_index, ImplicitParameterListNode{move(implicit_parameters)}
      ));
    }
    return None();
  }

  Option<NodeId> try_parse_function_signature_capture_annotation_list() {
    auto next_token = peek();
    if (next_token.type == TokenType::LEFT_BRACKET ||
        next_token.type == TokenType::LEFT_BRACKET_NO_W) {
      ++m_token_index; // consume the '[' token
      List<NodeId> capture_annotations;
      while (peek().type != TokenType::RIGHT_BRACKET) {
        capture_annotations.push_back(parse_function_signature_capture_annotation());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the ',' token
        }
      }
      ++m_token_index; // consume the ']' token
      return Some(m_output.add_node(
          next_token.id,
          m_token_index,
          FunctionSignatureCaptureAnnotationListNode{move(capture_annotations)}
      ));
    }
    return None();
  }

  NodeId parse_function_signature_capture_annotation() {
    auto start_token = peek();
    FunctionCaptureKind kind;
    auto token = next();
    if (token.type == TokenType::AMPERSAND) {
      kind = FunctionCaptureKind::Ref;
    } else if (token.type == TokenType::KEYWORD_MOVE) {
      kind = FunctionCaptureKind::Move;
    } else if (token.type == TokenType::KEYWORD_COPY) {
      kind = FunctionCaptureKind::Copy;
    } else {
      throw_parser_error(
          token.id, "Expected capture annotation to start with '&', 'move', or 'copy'"
      );
    }
    NodeId var = expect_identifier("Expected variable name in function capture annotation");
    return m_output.add_node(
        start_token.id, m_token_index, FunctionSignatureCaptureAnnotationNode{kind, var}
    );
  }

  Option<NodeId> try_parse_function_return_type() {
    auto next_token = peek();
    if (next_token.type == TokenType::ARROW) {
      ++m_token_index; // consume the '->' token
      return parse_expr();
    }
    return None();
  }

  NodeId parse_label_statement() {
    auto label_token = next();
    NodeId label = expect_identifier("Expected identifier after 'label' keyword in label statement"
    );
    return m_output.add_node(label_token.id, m_token_index, LabelStmtNode{label});
  }

  NodeId parse_goto_statement() {
    auto goto_token = next();
    NodeId label = expect_identifier("Expected identifier after 'goto' keyword in goto statement");
    return m_output.add_node(goto_token.id, m_token_index, GotoStmtNode{label});
  }

  NodeId parse_empty_statement() {
    auto semicolon_token = next();
    return m_output.add_node(semicolon_token.id, m_token_index, EmptyStmtNode{});
  }

  NodeId parse_while_statement() {
    auto while_token = next();
    read_left_paren("Expected '(' after 'while' keyword in while statement");
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expr();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in while statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        while_token.id, m_token_index, WhileStmtNode{move(introductory_decls), condition, body}
    );
  }

  NodeId parse_for_in_statement() {
    auto for_token = next();
    read_left_paren("Expected '(' after 'for' keyword in for-in statement");
    auto introductory_decls = parse_introductory_decls();
    List<NodeId> vars;
    vars.push_back(parse_for_in_variable());
    enforce_separator_after_introductory_decls(introductory_decls, vars[0]);
    while (peek().type == TokenType::COMMA) {
      ++m_token_index; // consume the ',' token
      vars.push_back(parse_for_in_variable());
    }
    read_token_type(TokenType::KEYWORD_IN, "Expected 'in' keyword in for-in statement");
    NodeId iterable = parse_expr();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after iterable in for-in statement");
    NodeId body = parse_statement();
    return m_output.add_node(
        for_token.id,
        m_token_index,
        ForInStmtNode{move(introductory_decls), move(vars), iterable, body}
    );
  }

  NodeId parse_for_in_variable() {
    auto name_token = peek();
    if (!is_identifier(name_token.type)) {
      throw_parser_error_at_current_location("Expected variable name in for-in statement");
    }
    NodeId name = parse_identifier();
    Option<NodeId> type_annotation;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_type_expr();
    }
    return m_output.add_node(
        name_token.id, m_token_index, ForInVariableNode{name, type_annotation}
    );
  }

  NodeId parse_throw_statement() {
    auto throw_token = next();
    Option<NodeId> expr;
    auto next_token = peek();
    if (next_token.loc.line == throw_token.loc.line && next_token.type != TokenType::SEMICOLON &&
        next_token.type != TokenType::END_OF_FILE && next_token.type != TokenType::RIGHT_BRACE) {
      expr = parse_expr();
    }
    return m_output.add_node(throw_token.id, m_token_index, ThrowStmtNode{expr});
  }

  NodeId parse_if_statement() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if' keyword in if statement");
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expr();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in if statement");
    NodeId then_branch = parse_statement();
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' keyword
      else_branch = parse_statement();
    }
    return m_output.add_node(
        if_token.id,
        m_token_index,
        IfStmtNode{move(introductory_decls), condition, then_branch, else_branch}
    );
  }

  NodeId parse_block_statement() {
    auto left_brace_token = read_token_type(
        TokenType::LEFT_BRACE, "Expected '{' at the beginning of block statement"
    );
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' at the end of block statement");
    return m_output.add_node(left_brace_token.id, m_token_index, BlockStmtNode{move(stmts)});
  }

  NodeId parse_pre_increment_statement() {
    auto token = next();
    NodeId operand = parse_expr();
    return m_output.add_node(token.id, m_token_index, PreIncrementStmtNode{operand});
  }

  NodeId parse_pre_decrement_statement() {
    auto token = next();
    NodeId operand = parse_expr();
    return m_output.add_node(token.id, m_token_index, PreDecrementStmtNode{operand});
  }

  NodeId parse_const_decl() {
    auto const_token = next();
    NodeId target = parse_expr();
    Option<NodeId> type_annotation;
    Option<NodeId> expr;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_type_expr();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expr = parse_expr();
    }
    return m_output.add_node(
        const_token.id, m_token_index, ConstDeclNode{target, type_annotation, expr}
    );
  }

  NodeId parse_let_decl() {
    auto let_token = next();
    NodeId target = parse_expr();
    Option<NodeId> type_annotation;
    Option<NodeId> expr;
    if (peek().type == TokenType::COLON) {
      ++m_token_index; // consume the ':' token
      type_annotation = parse_type_expr();
    }
    if (peek().type == TokenType::ASSIGN) {
      ++m_token_index; // consume the '=' token
      expr = parse_expr();
    }
    return m_output.add_node(
        let_token.id, m_token_index, LetDeclNode{target, type_annotation, expr}
    );
  }

  NodeId parse_expr_statement() {
    auto expr_token = peek();
    NodeId expr = parse_expr();
    auto next_token = peek();
    switch (next_token.type) {
    case TokenType::DOUBLE_PLUS_NO_W:
      ++m_token_index; // consume the '++' operator
      return m_output.add_node(expr_token.id, m_token_index, PostIncrementStmtNode{expr});
    case TokenType::DOUBLE_MINUS_NO_W:
      ++m_token_index; // consume the '--' operator
      return m_output.add_node(expr_token.id, m_token_index, PostDecrementStmtNode{expr});
    case TokenType::ASSIGN:
      ++m_token_index; // consume the '=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, AssignmentStmtNode{expr, parse_expr()}
      );
    case TokenType::PLUS_EQUAL:
      ++m_token_index; // consume the '+=' operator
      return m_output.add_node(expr_token.id, m_token_index, AddAssignStmtNode{expr, parse_expr()});
    case TokenType::MINUS_EQUAL:
      ++m_token_index; // consume the '-=' operator
      return m_output.add_node(expr_token.id, m_token_index, SubAssignStmtNode{expr, parse_expr()});
    case TokenType::STAR_EQUAL:
      ++m_token_index; // consume the '*=' operator
      return m_output.add_node(expr_token.id, m_token_index, MulAssignStmtNode{expr, parse_expr()});
    case TokenType::SLASH_EQUAL:
      ++m_token_index; // consume the '/=' operator
      return m_output.add_node(expr_token.id, m_token_index, DivAssignStmtNode{expr, parse_expr()});
    case TokenType::PERCENT_EQUAL:
      ++m_token_index; // consume the '%=' operator
      return m_output.add_node(expr_token.id, m_token_index, ModAssignStmtNode{expr, parse_expr()});
    case TokenType::LSHIFT_EQUAL:
      ++m_token_index; // consume the '<<=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, LeftShiftAssignStmtNode{expr, parse_expr()}
      );
    case TokenType::RSHIFT_EQUAL:
      ++m_token_index; // consume the '>>=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, RightShiftAssignStmtNode{expr, parse_expr()}
      );
    case TokenType::AMPERSAND_EQUAL:
      ++m_token_index; // consume the '&=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, BitwiseAndAssignStmtNode{expr, parse_expr()}
      );
    case TokenType::PIPE_EQUAL:
      ++m_token_index; // consume the '|=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, BitwiseOrAssignStmtNode{expr, parse_expr()}
      );
    case TokenType::CARET_EQUAL:
      ++m_token_index; // consume the '^=' operator
      return m_output.add_node(
          expr_token.id, m_token_index, BitwiseXorAssignStmtNode{expr, parse_expr()}
      );
    default:
      return m_output.add_node(expr_token.id, m_token_index, ExprStmtNode{expr});
    }
  }

  NodeId parse_expr() {
    return parse_descend_expr_or();
  }

  NodeId parse_descend_expr_or() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_and();
    while (peek().type == TokenType::OR) {
      ++m_token_index; // consume the '||' operator
      NodeId right = parse_descend_expr_and();
      left = m_output.add_node(start_token.id, m_token_index, OrExprNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_and() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_bitwise_or();
    while (peek().type == TokenType::AND) {
      ++m_token_index; // consume the '&&' operator
      NodeId right = parse_descend_expr_bitwise_or();
      left = m_output.add_node(start_token.id, m_token_index, AndExprNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_or() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_bitwise_xor();
    while (peek().type == TokenType::PIPE) {
      ++m_token_index; // consume the '|' operator
      NodeId right = parse_descend_expr_bitwise_xor();
      left = m_output.add_node(start_token.id, m_token_index, BitwiseOrExprNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_xor() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_bitwise_and();
    while (peek().type == TokenType::CARET) {
      ++m_token_index; // consume the '^' operator
      NodeId right = parse_descend_expr_bitwise_and();
      left = m_output.add_node(start_token.id, m_token_index, BitwiseXorExprNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_bitwise_and() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_eq_ne();
    while (peek().type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      NodeId right = parse_descend_expr_eq_ne();
      left = m_output.add_node(start_token.id, m_token_index, BitwiseAndExprNode{left, right});
    }
    return left;
  }

  NodeId parse_descend_expr_eq_ne() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_gt_lt();
    auto next_token = peek();
    while (next_token.type == TokenType::EQUAL || next_token.type == TokenType::NOT_EQUAL) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_gt_lt();
      if (next_token.type == TokenType::EQUAL) {
        left = m_output.add_node(start_token.id, m_token_index, EqualsExprNode{left, right});
      } else {
        left = m_output.add_node(start_token.id, m_token_index, NotEqualsExprNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_gt_lt() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_lshift_rshift();
    auto next_token = peek();
    while (next_token.type == TokenType::GREATER || next_token.type == TokenType::LESS ||
           next_token.type == TokenType::GREATER_EQUAL ||
           next_token.type == TokenType::LESS_EQUAL) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_lshift_rshift();
      if (next_token.type == TokenType::GREATER) {
        left = m_output.add_node(start_token.id, m_token_index, GreaterExprNode{left, right});
      } else if (next_token.type == TokenType::LESS) {
        left = m_output.add_node(start_token.id, m_token_index, LessExprNode{left, right});
      } else if (next_token.type == TokenType::GREATER_EQUAL) {
        left = m_output.add_node(start_token.id, m_token_index, GreaterEqualsExprNode{left, right});
      } else {
        left = m_output.add_node(start_token.id, m_token_index, LessEqualsExprNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_lshift_rshift() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_add_node();
    auto next_token = peek();
    while (next_token.type == TokenType::LSHIFT || next_token.type == TokenType::RSHIFT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_add_node();
      if (next_token.type == TokenType::LSHIFT) {
        left = m_output.add_node(start_token.id, m_token_index, LeftShiftExprNode{left, right});
      } else {
        left = m_output.add_node(start_token.id, m_token_index, RightShiftExprNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_add_node() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_mul_div_mod();
    auto next_token = peek();
    while (next_token.type == TokenType::PLUS || next_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_mul_div_mod();
      if (next_token.type == TokenType::PLUS) {
        left = m_output.add_node(start_token.id, m_token_index, AddExprNode{left, right});
      } else {
        left = m_output.add_node(start_token.id, m_token_index, SubtractExprNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_descend_expr_mul_div_mod() {
    auto start_token = peek();
    NodeId left = parse_descend_expr_await_ref_copy_move_inline();
    auto next_token = peek();
    while (next_token.type == TokenType::STAR || next_token.type == TokenType::SLASH ||
           next_token.type == TokenType::PERCENT) {
      ++m_token_index; // consume the operator
      NodeId right = parse_descend_expr_await_ref_copy_move_inline();
      if (next_token.type == TokenType::STAR) {
        left = m_output.add_node(start_token.id, m_token_index, MultiplyExprNode{left, right});
      } else if (next_token.type == TokenType::SLASH) {
        left = m_output.add_node(start_token.id, m_token_index, DivideExprNode{left, right});
      } else {
        left = m_output.add_node(start_token.id, m_token_index, ModuloExprNode{left, right});
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_type_expr() {
    return parse_descend_expr_await_ref_copy_move_inline(false);
  }

  NodeId parse_descend_expr_await_ref_copy_move_inline(bool allow_funcall = true) {
    auto start_token = peek();
    if (start_token.type == TokenType::KEYWORD_AWAIT) {
      ++m_token_index; // consume the 'await' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move_inline(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, AwaitExprNode{expr});
    } else if (start_token.type == TokenType::KEYWORD_MOVE) {
      ++m_token_index; // consume the 'move' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move_inline(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, MoveExprNode{expr});
    } else if (start_token.type == TokenType::KEYWORD_COPY) {
      ++m_token_index; // consume the 'copy' keyword
      NodeId expr = parse_descend_expr_await_ref_copy_move_inline(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, CopyExprNode{expr});
    } else if (start_token.type == TokenType::AMPERSAND) {
      ++m_token_index; // consume the '&' operator
      bool is_const = false;
      bool is_move = false;
      auto following_token = peek();
      if (following_token.type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      } else if (following_token.type == TokenType::KEYWORD_MOVE) {
        is_move = true;
        ++m_token_index; // consume the 'move' keyword
      }
      NodeId expr = parse_descend_expr_await_ref_copy_move_inline(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, RefExprNode{is_const, is_move, expr});
    }
    return parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
  }

  NodeId parse_descend_expr_pos_neg_deref_not_bitnot_ell(bool allow_funcall = true) {
    auto start_token = peek();
    if (start_token.type == TokenType::PLUS) {
      ++m_token_index; // consume the '+' operator
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, PositiveExprNode{expr});
    } else if (start_token.type == TokenType::MINUS) {
      ++m_token_index; // consume the '-' operator
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, NegativeExprNode{expr});
    } else if (start_token.type == TokenType::TILDE) {
      ++m_token_index; // consume the '~' operator
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, BitwiseNotExprNode{expr});
    } else if (start_token.type == TokenType::STAR) {
      ++m_token_index; // consume the '*' operator
      bool is_const = false;
      if (peek().type == TokenType::KEYWORD_CONST) {
        is_const = true;
        ++m_token_index; // consume the 'const' keyword
      }
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, DerefExprNode{is_const, expr});
    } else if (start_token.type == TokenType::EXCLAM ||
               start_token.type == TokenType::EXCLAM_NO_W) {
      ++m_token_index; // consume the '!' operator
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, NotExprNode{expr});
    } else if (start_token.type == TokenType::ELLIPSIS) {
      ++m_token_index; // consume the '...' operator
      NodeId expr = parse_descend_expr_pos_neg_deref_not_bitnot_ell(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, EllipsisExprNode{expr});
    }
    return parse_descend_expr_impl_any_async(allow_funcall);
  }

  NodeId parse_descend_expr_impl_any_async(bool allow_funcall = true) {
    auto start_token = peek();
    if (start_token.type == TokenType::KEYWORD_IMPL) {
      ++m_token_index; // consume the 'impl' keyword
      NodeId type_expr = parse_descend_expr_impl_any_async(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, ImplTypeExprNode{type_expr});
    } else if (start_token.type == TokenType::KEYWORD_ANY) {
      ++m_token_index; // consume the 'any' keyword
      NodeId type_expr = parse_descend_expr_impl_any_async(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, AnyTypeExprNode{type_expr});
    } else if (start_token.type == TokenType::KEYWORD_ASYNC) {
      ++m_token_index; // consume the 'async' keyword
      NodeId type_expr = parse_descend_expr_impl_any_async(allow_funcall);
      return m_output.add_node(start_token.id, m_token_index, AsyncExprNode{type_expr});
    }
    return parse_descend_expr_field_ix_funcall_scope_question_exclam(allow_funcall);
  }

  NodeId parse_scoped_name() {
    if (!is_identifier(peek().type)) {
      throw_parser_error_at_current_location("Expected identifier");
    }
    return parse_descend_expr_field_ix_funcall_scope_question_exclam(false, false, false);
  }

  NodeId parse_descend_expr_field_ix_funcall_scope_question_exclam(
      bool allow_funcall = true, bool allow_ix = true, bool allow_field = true
  ) {
    auto start_token = peek();
    auto left = parse_atom();
    auto next_token = peek();
    while (
        ((next_token.type == TokenType::DOT_NO_W || next_token.type == TokenType::NUMBER_FIELD) &&
         allow_field) ||
        (next_token.type == TokenType::LEFT_BRACKET_NO_W && allow_ix) ||
        (next_token.type == TokenType::LEFT_PAREN_NO_W && allow_funcall) ||
        next_token.type == TokenType::DOUBLE_COLON_NO_W ||
        next_token.type == TokenType::QUESTION_NO_W || next_token.type == TokenType::EXCLAM_NO_W
    ) {
      if (next_token.type == TokenType::DOT_NO_W) {
        ++m_token_index; // consume the '.' operator
        auto next_type = peek().type;
        if (!is_identifier_no_w(next_type) && next_type != TokenType::KEYWORD_OPERATOR &&
            next_type != TokenType::KEYWORD_TYPE && next_type != TokenType::KEYWORD_SUPER) {
          throw_parser_error_at_current_location(
              "Expected identifier immediately after '.' in field access expression"
          );
        }
        left = m_output.add_node(
            start_token.id, m_token_index, FieldAccessExprNode{left, parse_identifier()}
        );
      } else if (next_token.type == TokenType::NUMBER_FIELD) {
        m_token_index++; // consume the numeric field token
        left = m_output.add_node(
            start_token.id, m_token_index, NumericFieldAccessExprNode{left, next_token.id}
        );
      } else if (next_token.type == TokenType::LEFT_BRACKET_NO_W) {
        ++m_token_index; // consume the '[' operator
        List<NodeId> indices;
        while (peek().type != TokenType::RIGHT_BRACKET) {
          auto index_start_token = peek();
          Option<NodeId> name;
          if (is_identifier(peek().type) && peek(1).type == TokenType::ASSIGN) {
            name = parse_identifier();
            ++m_token_index; // consume the '=' token
          }
          NodeId value = parse_expr();
          if (peek().type == TokenType::COMMA) {
            ++m_token_index; // consume the comma
          }
          indices.push_back(
              m_output.add_node(index_start_token.id, m_token_index, IndexNode{name, value})
          );
        }
        ++m_token_index; // consume the ']' token
        left = m_output.add_node(
            start_token.id, m_token_index, IndexingExprNode{left, move(indices)}
        );
      } else if (next_token.type == TokenType::LEFT_PAREN_NO_W) {
        ++m_token_index; // consume the '(' operator
        List<NodeId> args;
        while (peek().type != TokenType::RIGHT_PAREN) {
          args.push_back(parse_function_call_argument());
          if (peek().type == TokenType::COMMA) {
            ++m_token_index; // consume the comma
          }
        }
        ++m_token_index; // consume the ')' operator
        left = m_output.add_node(
            start_token.id, m_token_index, FunctionCallExprNode{left, move(args)}
        );
      } else if (next_token.type == TokenType::DOUBLE_COLON_NO_W) {
        ++m_token_index; // consume the '::' operator
        auto next_type = peek().type;
        if (!is_identifier_no_w(next_type) && next_type != TokenType::KEYWORD_OPERATOR) {
          throw_parser_error_at_current_location(
              "Expected identifier immediately after '::' in scope resolution expression"
          );
        }
        left = m_output.add_node(
            start_token.id, m_token_index, ScopeResolutionExprNode{left, parse_identifier()}
        );
      } else if (next_token.type == TokenType::QUESTION_NO_W) {
        ++m_token_index; // consume the '?'
        left = m_output.add_node(start_token.id, m_token_index, QuestionMarkExprNode{left});
      } else if (next_token.type == TokenType::EXCLAM_NO_W) {
        ++m_token_index; // consume the '!'
        left = m_output.add_node(start_token.id, m_token_index, ExclamationMarkExprNode{left});
      } else {
        throw RuntimeError("unreachable");
      }
      next_token = peek();
    }
    return left;
  }

  NodeId parse_atom() {
    switch (peek().type) {
    case TokenType::IDENTIFIER:
    case TokenType::IDENTIFIER_NO_W:
    case TokenType::QUOTED_IDENTIFIER:
    case TokenType::QUOTED_IDENTIFIER_NO_W:
    case TokenType::KEYWORD_THIS_TYPE:
      if (is_start_of_lambda_expr()) {
        return parse_lambda_expr();
      }
    // fallthrough...
    case TokenType::KEYWORD_OPERATOR:
      return parse_identifier();
    case TokenType::STRING_LITERAL:
      return parse_string_literal();
    case TokenType::CHAR_LITERAL:
      return parse_char_literal();
    case TokenType::NUMBER:
    case TokenType::NUMBER_FIELD:
      return parse_number_literal();
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_PAREN_NO_W:
      if (is_start_of_lambda_expr()) {
        return parse_lambda_expr();
      }
      return parse_parenthesized_expr();
    case TokenType::LEFT_BRACKET:
    case TokenType::LEFT_BRACKET_NO_W:
      return parse_bracket_expr();
    case TokenType::LEFT_BRACE:
      return parse_brace_expr();
    case TokenType::KEYWORD_IF:
      return parse_if_expr();
    case TokenType::KEYWORD_TRY:
      return parse_try_expr();
    case TokenType::KEYWORD_SWITCH:
      return parse_switch_expr();
    case TokenType::KEYWORD_FUN:
      return parse_function_expr();
    case TokenType::KEYWORD_TRUE:
    case TokenType::KEYWORD_FALSE:
      return parse_boolean_literal();
    case TokenType::KEYWORD_THIS:
      return parse_this_literal();
    case TokenType::KEYWORD_SUPER:
      return parse_super_literal();
    case TokenType::KEYWORD_DEFAULT:
      return parse_default_literal();
    case TokenType::KEYWORD_BOOL:
    case TokenType::KEYWORD_BYTE:
    case TokenType::KEYWORD_SHORT:
    case TokenType::KEYWORD_INT:
    case TokenType::KEYWORD_LONG:
    case TokenType::KEYWORD_UBYTE:
    case TokenType::KEYWORD_USHORT:
    case TokenType::KEYWORD_UINT:
    case TokenType::KEYWORD_ULONG:
    case TokenType::KEYWORD_USIZE:
    case TokenType::KEYWORD_FLOAT:
    case TokenType::KEYWORD_DOUBLE:
    case TokenType::KEYWORD_BITINT:
    case TokenType::KEYWORD_UBITINT:
    case TokenType::KEYWORD_CHAR:
    case TokenType::KEYWORD_STR:
    case TokenType::KEYWORD_NULL:
      return parse_primitive_type();
    case TokenType::KEYWORD_AUTO:
      return parse_auto_type();
    case TokenType::KEYWORD_WITH:
      return parse_with_expr();
    case TokenType::KEYWORD_TYPEOF:
      return parse_typeof_expr();
    default:
      String err("Expected expression, got token ");
      m_token_formatter.format_token(err, m_token_index);
      throw_parser_error_at_current_location(move(err));
    }
  }

  NodeId parse_typeof_expr() {
    auto typeof_token = next();
    read_left_paren("Expected '(' after 'typeof' keyword in typeof expression");
    NodeId expr = parse_expr();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after expr in typeof expression");
    return m_output.add_node(typeof_token.id, m_token_index, TypeOfExprNode{expr});
  }

  NodeId parse_with_expr() {
    auto with_token = next();
    read_left_paren("Expected '(' after 'with' keyword in with expression");
    List<NodeId> args;
    while (peek().type != TokenType::RIGHT_PAREN) {
      args.push_back(parse_function_call_argument());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    }
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after arguments in with expression");
    NodeId body = parse_expr();
    return m_output.add_node(with_token.id, m_token_index, WithExprNode{move(args), body});
  }

  NodeId parse_auto_type() {
    auto auto_token = next();
    return m_output.add_node(auto_token.id, m_token_index, AutoTypeNode{});
  }

  NodeId parse_primitive_type() {
    auto type_token = next();
    return m_output.add_node(type_token.id, m_token_index, PrimitiveTypeNode{type_token.id});
  }

  NodeId parse_default_literal() {
    auto default_token = next();
    return m_output.add_node(default_token.id, m_token_index, DefaultLiteralNode{});
  }

  NodeId parse_this_literal() {
    auto this_token = next();
    return m_output.add_node(this_token.id, m_token_index, ThisLiteralNode{});
  }

  NodeId parse_super_literal() {
    auto super_token = next();
    return m_output.add_node(super_token.id, m_token_index, SuperLiteralNode{});
  }

  NodeId parse_this_type() {
    auto this_type_token = next();
    return m_output.add_node(this_type_token.id, m_token_index, ThisTypeNode{});
  }

  NodeId parse_boolean_literal() {
    auto bool_token = next();
    return m_output.add_node(
        bool_token.id, m_token_index, BooleanLiteralNode{bool_token.type == TokenType::KEYWORD_TRUE}
    );
  }

  NodeId parse_function_expr() {
    auto fun_token = next();
    NodeId signature = parse_function_signature();
    Option<NodeId> body;
    if (peek().type == TokenType::LEFT_BRACE) {
      body = try_parse_function_body();
    }
    return m_output.add_node(fun_token.id, m_token_index, FunctionExprNode{signature, body});
  }

  NodeId parse_operator_ident() {
    auto start_token = next(); // consume the 'operator' token
    NodeId operator_node;
    switch (next().type) {
    case TokenType::PLUS:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentAddNode{});
      break;
    case TokenType::MINUS:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentSubNode{});
      break;
    case TokenType::STAR:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentStarNode{});
      break;
    case TokenType::SLASH:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentDivNode{});
      break;
    case TokenType::PERCENT:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentModNode{});
      break;
    case TokenType::DOUBLE_PLUS:
    case TokenType::DOUBLE_PLUS_NO_W:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentIncNode{});
      break;
    case TokenType::DOUBLE_MINUS:
    case TokenType::DOUBLE_MINUS_NO_W:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentDecNode{});
      break;
    case TokenType::EQUAL:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentEqNode{});
      break;
    case TokenType::NOT_EQUAL:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentNeqNode{});
      break;
    case TokenType::GREATER:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentGtNode{});
      break;
    case TokenType::LESS:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentLtNode{});
      break;
    case TokenType::GREATER_EQUAL:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentGteNode{});
      break;
    case TokenType::LESS_EQUAL:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentLteNode{});
      break;
    case TokenType::EXCLAM:
    case TokenType::EXCLAM_NO_W:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentNotNode{});
      break;
    case TokenType::AND:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentAndNode{});
      break;
    case TokenType::OR:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentOrNode{});
      break;
    case TokenType::TILDE:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseNotNode{}
      );
      break;
    case TokenType::AMPERSAND:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentAmpersandNode{}
      );
      break;
    case TokenType::PIPE:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseOrNode{}
      );
      break;
    case TokenType::CARET:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseXorNode{}
      );
      break;
    case TokenType::LSHIFT:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentLeftShiftNode{}
      );
      break;
    case TokenType::RSHIFT:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentRightShiftNode{}
      );
      break;
    case TokenType::ASSIGN:
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentAssignNode{});
      break;
    case TokenType::PLUS_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentAddAssignNode{}
      );
      break;
    case TokenType::MINUS_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentSubAssignNode{}
      );
      break;
    case TokenType::STAR_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentMulAssignNode{}
      );
      break;
    case TokenType::SLASH_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentDivAssignNode{}
      );
      break;
    case TokenType::PERCENT_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentModAssignNode{}
      );
      break;
    case TokenType::AMPERSAND_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseAndAssignNode{}
      );
      break;
    case TokenType::PIPE_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseOrAssignNode{}
      );
      break;
    case TokenType::CARET_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentBitwiseXorAssignNode{}
      );
      break;
    case TokenType::LSHIFT_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentLeftShiftAssignNode{}
      );
      break;
    case TokenType::RSHIFT_EQUAL:
      operator_node = m_output.add_node(
          start_token.id, m_token_index, OperatorIdentRightShiftAssignNode{}
      );
      break;
    case TokenType::LEFT_BRACKET:
    case TokenType::LEFT_BRACKET_NO_W:
      read_token_type(TokenType::RIGHT_BRACKET, "Expected ']' following 'operator['");
      if (peek().type == TokenType::ASSIGN) {
        ++m_token_index; // consume the '=' token
        operator_node = m_output.add_node(
            start_token.id, m_token_index, OperatorIdentIxAssignNode{}
        );
      } else {
        operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentIxNode{});
      }
      break;
    case TokenType::LEFT_PAREN:
    case TokenType::LEFT_PAREN_NO_W:
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' following 'operator('");
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentFuncallNode{});
      break;
    case TokenType::KEYWORD_AS: {
      auto type = parse_type_expr();
      operator_node = m_output.add_node(start_token.id, m_token_index, OperatorIdentAsNode{type});
      break;
    }
    default:
      throw_parser_error_at_current_location("Expected operator after 'operator' keyword");
    }
    return m_output.add_node(start_token.id, m_token_index, OperatorIdentifierNode{operator_node});
  }

  NodeId parse_switch_expr() {
    auto switch_token = next();
    read_left_paren("Expected '(' after 'switch'");
    auto introductory_decls = parse_introductory_decls();
    NodeId expr = parse_expr();
    enforce_separator_after_introductory_decls(introductory_decls, expr);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after switch expression");
    read_token_type(TokenType::LEFT_BRACE, "Expected '{' to start switch expr body");
    List<NodeId> clauses;
    while (peek().type == TokenType::KEYWORD_CASE) {
      clauses.push_back(parse_switch_expr_case_clause());
    }
    Option<NodeId> default_body;
    if (peek().type == TokenType::KEYWORD_DEFAULT) {
      ++m_token_index; // consume the 'default' keyword
      default_body = parse_expr();
    }
    read_token_type(TokenType::RIGHT_BRACE, "Expected '}' to end switch expr body");
    return m_output.add_node(
        switch_token.id,
        m_token_index,
        SwitchExprNode{move(introductory_decls), expr, move(clauses), default_body}
    );
  }

  NodeId parse_switch_expr_case_clause() {
    auto case_token = next();
    NodeId header = parse_case_clause_header();
    NodeId body = parse_expr();
    return m_output.add_node(case_token.id, m_token_index, CaseClauseNode{header, body});
  }

  NodeId parse_case_clause_header() {
    auto start_token = peek();
    Option<List<NodeId>> introductory_decls;
    Option<List<NodeId>> exprs;

    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      ++m_token_index; // consume the left paren
      introductory_decls = parse_introductory_decls();
      exprs = List<NodeId>();
      do {
        exprs.value().push_back(parse_expr());
        if (peek().type == TokenType::COMMA) {
          ++m_token_index; // consume the comma
        }
      } while (peek().type != TokenType::RIGHT_PAREN);
      enforce_separator_after_introductory_decls(introductory_decls.value(), exprs.value()[0]);
      ++m_token_index; // consume the right paren
    }

    Option<NodeId> when_clause = try_parse_when_clause();

    if (!introductory_decls.has_value() && !exprs.has_value() && !when_clause.has_value()) {
      throw_parser_error(start_token.id, "Expected case clause header");
    }

    return m_output.add_node(
        start_token.id,
        m_token_index,
        CaseClauseHeaderNode{move(introductory_decls), move(exprs), when_clause}
    );
  }

  Option<NodeId> try_parse_when_clause() {
    Option<NodeId> when_clause;
    auto start_token = peek();
    if (start_token.type == TokenType::KEYWORD_WHEN) {
      ++m_token_index; // consume the 'when' keyword
      read_left_paren("Expected '(' after 'when' in case clause");
      auto introductory_decls = parse_introductory_decls();
      NodeId condition = parse_expr();
      enforce_separator_after_introductory_decls(introductory_decls, condition);
      when_clause = m_output.add_node(
          start_token.id, m_token_index, WhenClauseNode{move(introductory_decls), condition}
      );
      read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'when' clause");
    }
    return when_clause;
  }

  NodeId parse_try_expr() {
    auto try_token = next();
    NodeId try_block = parse_expr();
    List<NodeId> catch_clauses;
    while (peek().type == TokenType::KEYWORD_CATCH) {
      catch_clauses.push_back(parse_try_expr_catch_clause());
    }
    Option<NodeId> else_branch;
    if (peek().type == TokenType::KEYWORD_ELSE) {
      ++m_token_index; // consume the 'else' keyword
      else_branch = parse_expr();
    }
    return m_output.add_node(
        try_token.id, m_token_index, TryExprNode{try_block, move(catch_clauses), else_branch}
    );
  }

  NodeId parse_try_expr_catch_clause() {
    auto catch_token = next();
    read_left_paren("Expected '(' after 'catch'");
    Option<NodeId> var;
    if (is_identifier(peek().type) && peek(1).type == TokenType::COLON) {
      var = parse_identifier();
      ++m_token_index; // consume the ':' token
    }
    NodeId exc_type = parse_type_expr();
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after catch clause exception type");
    NodeId body = parse_expr();
    return m_output.add_node(catch_token.id, m_token_index, CatchClauseNode{exc_type, var, body});
  }

  NodeId parse_if_expr() {
    auto if_token = next();
    read_left_paren("Expected '(' after 'if'");
    List<NodeId> introductory_decls = parse_introductory_decls();
    NodeId condition = parse_expr();
    enforce_separator_after_introductory_decls(introductory_decls, condition);
    read_token_type(TokenType::RIGHT_PAREN, "Expected ')' after condition in 'if' expression");
    NodeId then_branch = parse_expr();
    read_token_type(
        TokenType::KEYWORD_ELSE, "Expected 'else' after then-branch of 'if' expression"
    );
    NodeId else_branch = parse_expr();
    return m_output.add_node(
        if_token.id,
        m_token_index,
        IfExprNode{move(introductory_decls), condition, then_branch, else_branch}
    );
  }

  NodeId parse_bracket_expr() {
    auto open_bracket = next();
    List<NodeId> exprs;
    parse_comma_separated_expr_list(exprs, TokenType::RIGHT_BRACKET);
    ++m_token_index; // consume the right bracket
    return m_output.add_node(open_bracket.id, m_token_index, BracketExprNode{move(exprs)});
  }

  NodeId parse_parenthesized_expr() {
    auto open_paren = next();
    List<NodeId> exprs;
    parse_comma_separated_expr_list(exprs, TokenType::RIGHT_PAREN);
    ++m_token_index; // consume the right paren
    return m_output.add_node(open_paren.id, m_token_index, ParenthesizedExprNode{move(exprs)});
  }

  NodeId parse_brace_expr() {
    auto next_token = peek(1);
    if (next_token.type == TokenType::END_OF_FILE) {
      throw_parser_error_at_current_location("Unexpected end of file after '{'. Expected object "
                                             "literal, object type, or block expr.");
    }
    if (next_token.type == TokenType::RIGHT_BRACE || next_token.type == TokenType::DOT) {
      return parse_object_literal();
    } else if (is_identifier(next_token.type) && peek(2).type == TokenType::COLON) {
      return parse_object_type();
    }
    return parse_block_expr();
  }

  NodeId parse_block_expr() {
    auto open_brace = next();
    List<NodeId> stmts;
    parse_statements(stmts, TokenType::RIGHT_BRACE);
    ++m_token_index; // consume the right brace
    return m_output.add_node(open_brace.id, m_token_index, BlockExprNode{move(stmts)});
  }

  NodeId parse_object_literal() {
    auto open_brace = next();
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      read_dot("Expected dot before field name in object literal");
      auto field_token = peek();
      auto field = expect_identifier("Expected field name immediately after dot in object literal");
      read_token_type(TokenType::ASSIGN, "Expected '=' after field name in object literal");
      NodeId value = parse_expr();
      entries.push_back(
          m_output.add_node(field_token.id, m_token_index, KeyValueEntryNode{field, value})
      );
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    }
    ++m_token_index; // consume the right brace
    return m_output.add_node(
        open_brace.id, m_token_index, AnonymousStructLiteralNode{move(entries)}
    );
  }

  NodeId parse_object_type() {
    auto open_brace = next();
    List<NodeId> entries;
    while (peek().type != TokenType::RIGHT_BRACE) {
      auto field_token = peek();
      auto field = expect_identifier("Expected field name in object literal");
      read_token_type(TokenType::COLON, "Expected ':' after field name in object type");
      NodeId type = parse_type_expr();
      entries.push_back(
          m_output.add_node(field_token.id, m_token_index, KeyValueEntryNode{field, type})
      );
      if (peek().type == TokenType::COMMA) {
        ++m_token_index; // consume the comma
      }
    }
    ++m_token_index; // consume the right brace
    return m_output.add_node(open_brace.id, m_token_index, AnonymousStructTypeNode{move(entries)});
  }

  void parse_comma_separated_expr_list(List<NodeId> &exprs, TokenType terminator) {
    while (peek().type != terminator) {
      exprs.push_back(parse_expr());
      if (peek().type == TokenType::COMMA) {
        ++m_token_index;
      }
    }
  }

  NodeId parse_string_literal() {
    auto token = next();
    return m_output.add_node(token.id, m_token_index, StringLiteralNode{token.id});
  }

  NodeId parse_char_literal() {
    auto token = next();
    return m_output.add_node(token.id, m_token_index, CharLiteralNode{token.id});
  }

  NodeId parse_number_literal() {
    auto token = next();
    return m_output.add_node(token.id, m_token_index, NumberLiteralNode{token.id});
  }

  NodeId expect_identifier(Text error_message) {
    auto token = peek();
    if (!is_identifier(token.type)) {
      throw_parser_error(token.id, String(error_message));
    }
    return parse_identifier();
  }

  NodeId parse_identifier() {
    if (peek().type == TokenType::KEYWORD_OPERATOR) {
      return parse_operator_ident();
    }
    return parse_single_identifier();
  }

  bool is_start_of_lambda_expr() {
    auto next_token = peek();
    if (is_identifier(next_token.type)) {
      return peek(1).type == TokenType::ARROW;
    } else if (next_token.type == TokenType::LEFT_PAREN ||
               next_token.type == TokenType::LEFT_PAREN_NO_W) {
      int lookahead = 1;
      next_token = peek(lookahead);
      while (next_token.type != TokenType::RIGHT_PAREN) {
        /*
          In a nutshell - in a parenthesized lambda parameter list, we should only see identifiers
          and commas. The only exception is the colon character, which can appear immediately after
          an identifier. If we see anything else, this isn't a lambda parameter list.
        */

        if (is_identifier(next_token.type)) {
          if (peek(lookahead + 1).type == TokenType::COLON) {
            return true;
          }
        } else if (next_token.type != TokenType::COMMA) {
          return false;
        }

        ++lookahead;
        next_token = peek(lookahead);
      }
      return peek(lookahead + 1).type == TokenType::ARROW;
    }
    return false;
  }

  NodeId parse_lambda_expr() {
    auto start_token = peek();
    if (start_token.type == TokenType::LEFT_PAREN ||
        start_token.type == TokenType::LEFT_PAREN_NO_W) {
      List<NodeId> params = parse_function_parameter_list();
      read_token_type(TokenType::ARROW, "Expected '->' after lambda parameter");
      NodeId body = parse_expr();
      return m_output.add_node(start_token.id, m_token_index, LambdaExprNode{move(params), body});
    }

    auto single_param = expect_identifier(
        "Expected identifier or '(' to start lambda parameter list"
    );
    read_token_type(TokenType::ARROW, "Expected '->' after lambda parameter");
    NodeId body = parse_expr();
    return m_output.add_node(
        start_token.id, m_token_index, LambdaExprNode{List({single_param}), body}
    );
  }

  NodeId parse_single_identifier() {
    auto ident = next();
    return m_output.add_node(ident.id, m_token_index, IdentifierNode{ident.id});
  }

  NodeId parse_function_call_argument() {
    Option<NodeId> name;
    auto next_token = peek();
    if (is_identifier(next_token.type) && peek(1).type == TokenType::ASSIGN) {
      name = parse_identifier();
      ++m_token_index; // consume the '=' token
    }
    NodeId expr = parse_expr();
    return m_output.add_node(next_token.id, m_token_index, FunctionArgumentNode{name, expr});
  }

private:
  TokenWithId read_dot(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::DOT && token.type != TokenType::DOT_NO_W) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId read_left_paren(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::LEFT_PAREN && token.type != TokenType::LEFT_PAREN_NO_W) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId read_left_bracket(Text error_message) {
    auto token = peek();
    if (token.type != TokenType::LEFT_BRACKET && token.type != TokenType::LEFT_BRACKET_NO_W) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId read_token_type(TokenType expected, Text error_message) {
    auto token = peek();
    if (token.type != expected) {
      throw_parser_error(token.id, String(error_message));
    }
    ++m_token_index;
    return token;
  }

  TokenWithId next() {
    if (m_token_index >= static_cast<TokenId>(m_input.tokens().size())) {
      throw RuntimeError("Attempting to read past end of token stream");
    }
    auto token = m_input.get_token(m_token_index);
    TokenWithId result{m_token_index, token.type, token.location};
    ++m_token_index;
    return result;
  }

  TokenWithId peek(TokenId n = 0) const {
    if (m_token_index + n >= static_cast<TokenId>(m_input.tokens().size())) {
      throw RuntimeError("Attempting to peek past end of token stream");
    }
    auto token = m_input.get_token(m_token_index + n);
    return TokenWithId{m_token_index + n, token.type, token.location};
  }

  [[noreturn]] void throw_parser_error_at_current_location(String message) const {
    throw_parser_error(m_token_index, move(message));
  }

  [[noreturn]] void throw_parser_error_at_location(Location location, String message) const {
    throw ParserError(location, move(message));
  }

  [[noreturn]] void throw_parser_error(TokenId token_id, String message) const {
    auto token = m_input.get_token(token_id);
    throw ParserError(token.location, move(message));
  }

  ParserResult &m_output;
  const LexerResult &m_input;
  TokenId m_token_index;
  TokenFormatter m_token_formatter;

  List<NodeId> m_imports;
  List<NodeId> m_submodules;
};
} // namespace

NodeId Parser::parse_module(ParserResult &output, const LexerResult &input) {
  ParserState state(output, input);
  return state.parse_module();
}

} // namespace amelia
