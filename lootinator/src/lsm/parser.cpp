#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <algorithm> 
#include <cctype>   
#include <unordered_map>
#include <cassert>

#include "lootinator/lsm/parser.hpp"

namespace loot {
    namespace lsm {
        std::unordered_map<std::string, loot::lsm::Token> token_lookup = {
            {"assert", loot::lsm::Token(loot::lsm::TokenType::ASSERT)}, 
            {"item", loot::lsm::Token(loot::lsm::TokenType::ITEM)},
            {"count-items", loot::lsm::Token(loot::lsm::TokenType::COUNT_ITEMS)},
            {"item-count", loot::lsm::Token(loot::lsm::TokenType::ITEM_COUNT)},
            {"pool", loot::lsm::Token(loot::lsm::TokenType::POOL)},
            {"roll", loot::lsm::Token(loot::lsm::TokenType::ROLL)}
        };

        loot::lsm::Token::Token(loot::lsm::TokenType tp, std::string string_repr) {
            this->tp = tp;
            this->string_repr = string_repr;
        }

        loot::lsm::Token::Token(loot::lsm::TokenType tp, uint32_t int_repr) {
            this->tp = tp;
            this->int_repr = int_repr;
        }

        loot::lsm::Token::Token(loot::lsm::TokenType tp) {
            this->tp = tp;
            this->int_repr = int_repr;
        }

        void loot::lsm::Token::string() {
            switch (this->tp) {
                case NUMBER:
                    printf("Number{%u}\n", this->int_repr);
                    break;
                case IDENTIFIER:
                    printf("Identifier{%s}\n", this->string_repr.c_str());
                    break;
                case OPEN_CURLY_BRACE:
                    printf("{\n");
                    break;
                case CLOSE_CURLY_BRACE:
                    printf("}\n");
                    break;
                case GREATER_EQUAL:
                    printf(">=\n");
                    break;
                case GREATER:
                    printf(">\n");
                    break;
                case SEMI_COLON:
                    printf("Semicolon\n");
                    break;
                case OPEN_BRACKET:
                    printf("(\n");
                    break;
                case CLOSE_BRACKET:
                    printf(")\n");
                    break;
                case COUNT_ITEMS:
                    printf("CountItems\n");
                    break;
                case ITEM:
                    printf("Item\n");
                    break;
                case ASSERT:
                    printf("Assert\n");
                    break;
                case ITEM_COUNT:
                    printf("ItemCount\n");
                    break;
                case POOL:
                    printf("Pool\n");
                    break;
                case ROLL:
                    printf("Roll\n");
                    break;
                default:
                    printf("Token not found!\n");
                    exit(1);
            }
        }

        std::string ltrim(const std::string& s) {
            size_t start = s.find_first_not_of(" \t\n\r\f\v");
            return (start == std::string::npos) ? "" : s.substr(start);
        }

        // Function to trim trailing whitespace
        std::string rtrim(const std::string& s) {
            size_t end = s.find_last_not_of(" \t\n\r\f\v");
            return (end == std::string::npos) ? "" : s.substr(0, end + 1);
        }

        // Function to trim both leading and trailing whitespace
        std::string trim(const std::string& s) {
            return ltrim(rtrim(s));
        }

        loot::lsm::Token loot::lsm::Tokenizer::next_token() {
            return this->tokens[this->token_index++];
        }

        bool loot::lsm::Tokenizer::next_token_is(loot::lsm::TokenType tp) {
            Token t = this->next_token();
            return t.tp == tp;
        }

        loot::lsm::Tokenizer::Tokenizer(std::string buffer) {
            std::string::iterator new_end = std::unique(buffer.begin(), buffer.end(),
                [=](char lhs, char rhs){ return (lhs == rhs) && (lhs == ' '); }
            );
            buffer.erase(new_end, buffer.end());
            buffer.erase(std::remove(buffer.begin(), buffer.end(), '\n'), buffer.end());
            for (size_t i = 0; i < buffer.length(); i++) {
                // printf("%d\n", i);
                char character = buffer[i];
                // std::cout << character << "\n";
                switch (character) {
                    case ' ':
                        break;
                    case ';':
                        this->tokens.push_back(Token(loot::lsm::SEMI_COLON));
                        break;
                    case '{':   
                        this->tokens.push_back(Token(loot::lsm::OPEN_CURLY_BRACE));
                        break;
                    case '}':   
                        this->tokens.push_back(Token(loot::lsm::CLOSE_CURLY_BRACE));
                        break;
                    case '(':   
                        this->tokens.push_back(Token(loot::lsm::OPEN_BRACKET));
                        break;
                    case ')':   
                        this->tokens.push_back(Token(loot::lsm::CLOSE_BRACKET));
                        break;
                    case '>': {
                        if (buffer[i + 1] == '=') {
                            this->tokens.push_back(Token(loot::lsm::GREATER_EQUAL));
                            i++;
                            break;
                        }
                        this->tokens.push_back(Token(loot::lsm::GREATER));
                        break;
                    }
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': {
                        std::string repr;
                        while (isdigit(character)) {
                            repr += character;
                            i++;
                            character = buffer[i];
                        }
                        this->tokens.push_back(Token(loot::lsm::NUMBER, std::stoll(repr)));
                        i--;
                        break;
                    }
                    default:  {
                        std::string repr;
                        while ((isalnum(character) || character == ':' || character == '_' || character == '-') && i < buffer.length()) {
                            repr += character;
                            i++;
                            character = buffer[i];
                        }
                        repr = trim(repr);
                        // std::cout << repr <<"\n";
                        i--;
                        try {
                            this->tokens.push_back(token_lookup.at(repr));
                        }
                        catch (...) {
                            this->tokens.push_back(Token(loot::lsm::IDENTIFIER, repr));
                        }
                        break;
                    }
                }
            }
        }

        loot::lsm::BlockInstruction parse_block(Tokenizer &tokenizer) {
            loot::lsm::BlockInstruction block;

            // loot::lsm::Token token = tokenizer.next_token();
            // token.string();
            return block;
        }

        loot::lsm::PoolInstruction parse_pool(Tokenizer &tokenizer) {
            loot::lsm::Token token = tokenizer.next_token();
            assert(token.tp == loot::lsm::TokenType::NUMBER);
            loot::lsm::PoolInstruction pool(token.int_repr);
            assert(tokenizer.next_token_is(loot::lsm::TokenType::SEMI_COLON));
            assert(tokenizer.next_token_is(loot::lsm::TokenType::OPEN_CURLY_BRACE));
            
            token = tokenizer.next_token();
            token.string();
            switch (token.tp) {
                case loot::lsm::TokenType::ROLL: {
                    printf("Todo: implement roll!\n");
                    break;
                }
                default: {
                    assert(false && "token has not been implemented yet!");
                    break;
                }
            }
            return pool;
        }

        void parse_tokens(Tokenizer& tokenizer, std::vector<loot::lsm::Instruction *> &instructions) {
            loot::lsm::Token token = tokenizer.next_token();
            switch (token.tp) {
                case loot::lsm::POOL: {
                    loot::lsm::PoolInstruction pool = parse_pool(tokenizer);
                    instructions.push_back(static_cast<loot::lsm::Instruction *>(&pool));
                    break;
                }
                default:
                    printf("instruction not handled yet!\n");
                    break;
            }
        }

        std::vector<loot::lsm::Instruction *> loot::lsm::Parser::parse_from_file(std::ifstream& file) {
            std::stringstream buffer;
            buffer << file.rdbuf(); // reading data
            std::string str = buffer.str();
            
            Tokenizer tokenizer = Tokenizer(str);
            
            std::vector<loot::lsm::Instruction *> instructions;
            parse_tokens(tokenizer, instructions);
            
            return instructions;
        }
    }   
}