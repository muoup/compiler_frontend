#include "lex.h"

#include <algorithm>
#include <format>
#include <stack>

#include "derive_lex.h"

using namespace lex;

std::optional<derived_lex> derive(const std::string_view span) {
    const auto try_derive = [span](auto fn) {
        return [span, fn] { return fn(span); };
    };

    return derive_operator(span)
        .or_else(try_derive(derive_punctuator))
        .or_else(try_derive(derive_strlit))
        .or_else(try_derive(derive_charlit));
}

void output_buffer(const auto buffer_start, const auto buffer_end, std::vector<lex_token>& tokens) {
    if (buffer_start == buffer_end)
        return;

    const auto shaved_start = std::find_if_not(buffer_start, buffer_end, iswspace);
    const std::string_view shaved_buffer = { shaved_start, buffer_end };

    if (shaved_buffer.empty())
        return;

    if (isdigit(*buffer_start))
        tokens.push_back(generate_numeric(shaved_buffer));
    else if (KEYWORD_SET.contains(shaved_buffer))
        tokens.emplace_back(lex_type::KEYWORD, shaved_buffer);
    else
        tokens.emplace_back(lex_type::IDENTIFIER, shaved_buffer);
}

void connect_punctuators(std::vector<lex_token>& tokens) {
    std::stack<ast::lex_ptr> stack;

    const auto punc_assert = [](const ast::lex_ptr token, const std::string_view val) {
        if (token->span != val)
            throw std::runtime_error("Mismatched punctuators");
    };

    for (auto ptr = tokens.begin(); ptr != tokens.end(); ++ptr) {
        if (ptr->type != lex_type::PUNCTUATOR)
            continue;

        switch (ptr->span.front()) {
            case '{':
            case '(':
            case '[':
                stack.emplace(ptr);
                continue;
            case '}':
                punc_assert(stack.top(), "{");
                break;
            case ')':
                punc_assert(stack.top(), "(");
                break;
            case ']':
                punc_assert(stack.top(), "[");
                break;
            default:
                throw std::runtime_error(std::format("Invalid Punctuator: {}", ptr->span));
        }

        stack.top()->closer = ptr;
        stack.pop();
    }
}

std::vector<lex_token> lex::lex(const std::string_view code) {
    std::vector<lex_token> tokens;
    auto buffer_start = code.begin();

    for (auto ptr = code.cbegin(); ptr != code.cend(); ++ptr) {
        if (*ptr == ' ' || *ptr == '\n') {
            output_buffer(buffer_start, ptr, tokens);
            buffer_start = ptr + 1;
            continue;
        }

        if (const auto derived = derive({ ptr, code.cend() })) {
            output_buffer(buffer_start, ptr, tokens);

            tokens.emplace_back(derived->type, derived->span);
            ptr = derived->end;
            buffer_start = ptr + 1;
        }
    }

    // TODO: If performance is bad, this can be done in the loop above to avoid the extra iteration
    connect_punctuators(tokens);

    return tokens;
}
