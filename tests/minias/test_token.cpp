#define CATCH_CONFIG_MAIN

#include <stdlib.h>
#include <string.h>
#include <catch2/catch_test_macros.hpp>
#include <_private/minias/token.h>

TEST_CASE("Malloc Token", "[minias][token]") {
    token_t *t = init_token(NULL);
    REQUIRE(t != NULL);

    REQUIRE(strcmp(t->span, "") == 0);
    REQUIRE(t->length == 0);
    REQUIRE(t->token_type == UNKNOWN_TOKEN);
    REQUIRE(t->line_no == 0);
    REQUIRE(t->col_no == 0);

    free(t);
}

TEST_CASE("Make EOF Token", "[minias][token]") {
    token_t t;
    init_token(&t);
    REQUIRE(make_eof_token(&t) == 0);

    REQUIRE(strcmp(t.span, "") == 0);
    REQUIRE(t.length == 0);
    REQUIRE(t.token_type == EOF_TOKEN);
    REQUIRE(t.line_no == 0);
    REQUIRE(t.col_no == 0);
}

void __no_problemo_125__(token_t *t) {
    REQUIRE(strcmp(t->span, "rbp") == 0);
    REQUIRE(t->length == 3);
    REQUIRE(t->token_type == IDENTIFIER_TOKEN);
    REQUIRE(t->line_no == 10);
    REQUIRE(t->col_no == 3);
}

TEST_CASE("Copy Tokens", "[minias][token]") {
    token_t dest, src, control;
    init_token(&dest);
    init_token(&src);

    strcpy(src.span, "rbp");
    src.length = 3;
    src.token_type = IDENTIFIER_TOKEN;
    src.line_no = 10;
    src.col_no = 3;

    control = src;
    REQUIRE(copy_token(&dest, &src) == &dest);

    __no_problemo_125__(&src);
    __no_problemo_125__(&dest);
    __no_problemo_125__(&control);
}
