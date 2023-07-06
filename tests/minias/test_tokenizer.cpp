#define CATCH_CONFIG_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <catch2/catch_test_macros.hpp>
#include <_private/minias/tokenizer.h>
#include <_test/config.h>

void __check_tok_sequence__(tokenizer_t *t, token_type_t *t_ty, std::string *span, size_t len) {
    int status = 0;
    token_t token;
    size_t index = 0;
    init_token(&token);
    while (status != EOF && token.token_type != EOF_TOKEN) {
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
    FILE *asm_file = fopen(one_liner_min, "r");
    set_tokenizer_file(tokenizer, asm_file);
    __check_tok_sequence__(tokenizer, t_ty, spans, 5);

    free(tokenizer);
}
