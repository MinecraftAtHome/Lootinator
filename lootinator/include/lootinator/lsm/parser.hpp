#ifndef LOOTINATOR_LSM_PARSER_H
#define LOOTINATOR_LSM_PARSER_H

#include <iostream>

#include "lootinator/lsm/instructions.hpp"

namespace lsm {
    enum TokenType {
        NUMBER, IDENTIFIER, OPEN_CURLY_BRACE, CLOSE_CURLY_BRACE, SEMI_COLON, OPEN_BRACKET, CLOSE_BRACKET, GREATER, LESS, GREATER_EQUAL, LESS_EQUAL,
        ASSERT, ITEM, COUNT_ITEMS, ITEM_COUNT, POOL, ROLL
    };

    struct Token {
        public:
            Token(TokenType tp, std::string string_repr);
            Token(TokenType tp, uint32_t int_repr);
            Token(TokenType tp);
            TokenType tp;
            std::string string_repr;
            uint32_t int_repr;
            void string();
    };

    class Tokenizer {
        private:
            int token_index = 0;
        public:
            std::vector<Token> tokens;
            
            Tokenizer(std::string);
            Token next_token();
            bool next_token_is(TokenType tp);
    };

    class Parser {
        private:
        public:
            std::vector<lsm::Instruction *> parse_from_file(std::ifstream& file);
    };
}

#endif