#define CATCH_CONFIG_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <catch2/catch_test_macros.hpp>
#include <_private/minias/token.h>
#include <_private/minias/tokenizer.h>
#include <_test/config.h>

void __check_tok_sequence__(
    tokenizer_t *t,
    token_type_t *t_ty,
    std::string *span,
    size_t len,
    bool truncate_at_len // stop looking for tokens after `len` number of tokens read
) {
    int status = 0;
    token_t token;
    size_t index = 0;
    init_token(&token);
    while (status != EOF && token.token_type != EOF_TOKEN) {
        if (index >= len)
            return;
        REQUIRE(index <= len); // require extra iteration for EOF
        status = get_next_token(t, &token);
        printf(" [%d] %s\n", token.token_type, token.span);
        REQUIRE((status == 0 || status == EOF));
        if (status == EOF) {
            break;
        }
        CHECK(token.token_type == t_ty[index]);
        CHECK(span[index] == token.span);
        index++;
    }
    REQUIRE(index == len);
}

TEST_CASE("Tokenizer one_liner.min", "[minias][tokenizer]") {
    tokenizer_t *tokenizer = init_tokenizer(NULL);
    REQUIRE(tokenizer != NULL);

    token_type_t t_ty[] = {
        IDENTIFIER_TOKEN,
        IDENTIFIER_TOKEN,
        COMMA_TOKEN,
        INT_TOKEN,
        NEWLINE_TOKEN
    };
    std::string spans[] = {
        "setub",
        "r0",
        ",",
        "255",
        "\n"
    };

    const char *one_liner_min = MINITEST_FODDER_DIR "/one_liner.min";
    printf("one_liner_min: %s\n", one_liner_min);
    FILE *asm_file = fopen(one_liner_min, "r");
    set_tokenizer_file(tokenizer, asm_file);
    __check_tok_sequence__(tokenizer, t_ty, spans, 5, false);

    free(tokenizer);
}

TEST_CASE("Tokenizer big_example.min", "[minias][tokenizer]") {
    tokenizer_t *tokenizer = init_tokenizer(NULL);
    REQUIRE(tokenizer != NULL);
    
    token_type_t t_ty[] = {
        IDENTIFIER_TOKEN, COLON_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, INT_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, COMMA_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, IDENTIFIER_TOKEN, NEWLINE_TOKEN,
        NEWLINE_TOKEN,
        IDENTIFIER_TOKEN, COLON_TOKEN, NEWLINE_TOKEN
    };
    std::string spans[] = {
        "main", ":", "\n",
        "pshq", "rbp", "\n",
        "movq", "rbp", ",", "rsp", "\n",
        "\n",
        "setiq", "r0", ",", "10", "\n",
        "setiq", "r1", ",", "1", "\n",
        "setiq", "r2", ",", "2", "\n",
        "setiq", "r3", ",", "3", "\n",
        "setiq", "r4", ",", "4", "\n",
        "setiq", "r5", ",", "5", "\n",
        "setiq", "r6", ",", "10", "\n",
        "pshq", "r6", "\n",
        "setiq", "r6", ",", "9", "\n",
        "pshq", "r6", "\n",
        "setiq", "r6", ",", "8", "\n",
        "pshq", "r6", "\n",
        "setiq", "r6", ",", "7", "\n",
        "pshq", "r6", "\n",
        "setiq", "r6", ",", "6", "\n",
        "pshq", "r6", "\n",
        "\n",
        "\n",
        "movq", "r6", ",", "rip", "\n",
        "setuq", "r7", ",", "26", "\n",
        "adduq", "r6", ",", "r7", "\n",
        "pshq", "r6", "\n",
        "setuq", "rip", ",", "sum", "\n",
        "\n",
        "\n",
        "\n",
        "movq", "rsp", ",", "rbp", "\n",
        "popq", "rbp", "\n",
        "popq", "rip", "\n",
        "\n",
        "sum", ":", "\n",
    };

    const char *big_example_min = MINITEST_FODDER_DIR "/big_example.min";
    printf("big_example_min: %s\n", big_example_min);
    FILE *asm_file = fopen(big_example_min, "r");
    set_tokenizer_file(tokenizer, asm_file);
    size_t stuff_to_check_len = sizeof(t_ty) / sizeof(t_ty[0]);
    __check_tok_sequence__(tokenizer, t_ty, spans, stuff_to_check_len, true);

    free(tokenizer);
}
