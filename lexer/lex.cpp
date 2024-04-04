#include "lex.h"

#include <algorithm>
#include <format>
#include <stack>

#include "derive_lex.h"

using namespace lex;

std::optional<derived_lex> derive(const str_ptr start, const str_ptr end) {
    const auto try_derive = [start, end](auto fn) {
        return [start, end, fn] { return fn(start, end); };
    };

    return derive_expr_op(start, end)
        .or_else(try_derive(derive_assn_op))
        .or_else(try_derive(derive_punctuator))
        .or_else(try_derive(derive_charlit))
        .or_else(try_derive(derive_strlit));
}

void output_buffer(const auto buffer_start, const auto buffer_end, std::vector<lex_token>& tokens) {
    if (buffer_start == buffer_end)
        return;

    const auto shaved_start = std::find_if_not(buffer_start, buffer_end, iswspace);
    const std::string_view shaved_buffer = { shaved_start, buffer_end };

    if (shaved_buffer.empty())
        return;

    if (isdigit(*buffer_start))
        tokens.push_back(gen_numeric(buffer_start, buffer_end));
    else if (KEYWORD_SET.contains(shaved_buffer))
        tokens.emplace_back(lex_type::KEYWORD, shaved_buffer);
    else if (PRIMITIVES_SET.contains(shaved_buffer))
        tokens.emplace_back(lex_type::PRIMITIVE, shaved_buffer);
    else
        tokens.emplace_back(lex_type::IDENTIFIER, shaved_buffer);
}

void connect_punctuators(std::vector<lex_token>& tokens) {
    std::stack<lex_ptr> stack;

    const auto punc_assert = [](const lex_ptr token, const std::string_view val) {
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
    auto buffer_start = code.cbegin();
    const auto end = std::cend(code);

    for (auto ptr = std::cbegin(code); ptr != end; ++ptr) {
        if (*ptr == ' ' || *ptr == '\n' || *ptr == '\0') {
            output_buffer(buffer_start, ptr, tokens);
            buffer_start = ptr + 1;
        }

        // Comments
        else if (*ptr == '/' && *(ptr + 1) == '/') {
            output_buffer(buffer_start, ptr, tokens);
            buffer_start = std::find(ptr, end, '\n');
            ptr = buffer_start;
        }

        else if (*ptr == '/' && *(ptr + 1) == '*') {
            output_buffer(buffer_start, ptr, tokens);
            while (ptr++ != end - 2 && (*ptr != '*' || *(ptr + 1) != '/')) {}
            buffer_start = ptr + 2;
            ptr += 2;
        }

        else if (const auto derived = derive(ptr, std::cend(code))) {
            output_buffer(buffer_start, ptr, tokens);

            tokens.emplace_back(derived->type, derived->span);
            ptr = derived->end;
            buffer_start = ptr + 1;
        }
    }

    output_buffer(buffer_start, end, tokens);

    // TODO: If performance is bad, this can be done in the loop above to avoid the extra iteration
    connect_punctuators(tokens);

    return tokens;
}
