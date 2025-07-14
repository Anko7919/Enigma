#ifndef DORAYAKI_ENIGMA_HPP_INCLUDED
#define DORAYAKI_ENIGMA_HPP_INCLUDED

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dorayaki {
    struct SetStringVisitor {
        std::set<char> operator()(const std::string &); 
        std::set<char> operator()(const std::set<char> &); 
    }; 

    template <std::size_t scramblers_num>
    class Enigma {
    public: 
        using array_value_type = std::int32_t;  ///< scramblersやrefrectorの要素の型

    private: 
        const std::set<char> available_characters; ///< 使用可能な文字
        std::size_t scramblers_pos; ///< スクランブラーの位置
        std::array<std::vector<array_value_type>, scramblers_num> scramblers;   ///< スクランブラーの設定
        std::vector<array_value_type> refrector;    ///< リフレクター

    public: 
        Enigma() = delete; 

        Enigma(std::initializer_list<std::mt19937::result_type> seeds, std::size_t startpos=0, std::variant<std::string, std::set<char>> avach="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890-^@[;:],./\\!\"#$%&'()==~~|`{+*<>?_} ")
                noexcept : available_characters(std::visit(SetStringVisitor{}, avach)), scramblers_pos(startpos % this->available_characters.size()), refrector(std::visit(SetStringVisitor{}, avach).size()) {

            // scramblersとrefrectorを[0, scramblers_num]で初期化
            std::iota(this->refrector.begin(), this->refrector.end(), 0); 
            this->scramblers.fill(this->refrector); 

            if (seeds.size() == 0) {
                return; 
            }
            // scramblersを設定
            for (std::size_t i = 0; i < scramblers_num; ++i) {
                std::mt19937 rnd{ *(seeds.begin() + (i % seeds.size())) }; 
                std::shuffle(scramblers.at(i).begin(), scramblers.at(i).end(), rnd); 
            }
            for (std::size_t i = 0; i < scramblers_num * this->available_characters.size(); ++i) {
                array_value_type &v = this->scramblers.at(i % scramblers_num).at(i / scramblers_num); 
                v = v - i / scramblers_num;  
            }
            std::vector<array_value_type> ref_copy{ this->refrector }; 
            std::mt19937 rnd{ *(seeds.begin() + scramblers_num % seeds.size()) }; 
            std::shuffle(ref_copy.begin(), ref_copy.end(), rnd); 
            for (std::size_t i = 0; i < this->refrector.size() / 2; ++i) {
                this->refrector.at(ref_copy.at(2 * i)) = ref_copy.at(2 * i + 1); 
                this->refrector.at(ref_copy.at(2 * i + 1)) = ref_copy.at(2 * i); 
            }
        }

        const std::set<char> &get_available_characters() const noexcept {
            return this->available_characters; 
        }

        const std::array<std::vector<array_value_type>, scramblers_num> &get_scramblers() const noexcept {
            return this->scramblers; 
        }
        
        const std::vector<array_value_type> &get_refrector() const noexcept {
            return this->refrector; 
        }

        std::size_t get_scramblers_position() const noexcept {
            return this->scramblers_pos; 
        }

        std::optional<std::size_t> calc_scrambler_position(std::size_t ind) {
            if (ind >= scramblers_num) {
                return std::nullopt; 
            }
            std::size_t p = this->scramblers_pos; 
            for (std::size_t i = 0; i < ind; ++i) {
                p = p / this->available_characters.size(); 
            }
            return p % this->available_characters.size(); 
        }

        Enigma<scramblers_num> &operator++() noexcept {
            ++(this->scramblers_pos); 
            return *this; 
        }
        
        Enigma<scramblers_num> operator++(int) noexcept {
            (this->scramblers_pos)++; 
            return *this; 
        }

        std::optional<char> operator()(char ch) {
            std::set<char>::const_iterator p{ this->available_characters.find(ch) }; 
            if (p == this->available_characters.cend()) {
                return std::nullopt; 
            }
            std::set<char>::const_iterator::difference_type ind = std::distance(this->available_characters.cbegin(), p); 
            std::size_t cardinality = this->available_characters.size(); 
            // スクランブラーを通過
            for (std::size_t i = 0; i < scramblers_num; ++i) {
                ind += this->scramblers.at(i).at((ind + this->calc_scrambler_position(i).value() + cardinality) % cardinality); 
                ind = (ind + cardinality) % cardinality; 
            }
            // リフレクターを通過
            ind = this->refrector.at(ind); 
            // スクランブラーを通過 (逆順)
            for (std::size_t i = 1; i <= scramblers_num; ++i) {
                for (std::size_t j = 0; j < cardinality; ++j) {
                    if ((j + this->scramblers.at(scramblers_num - i).at((j + this->calc_scrambler_position(scramblers_num - i).value() + cardinality) % cardinality) + cardinality) % cardinality == ind) {
                        ind = j; 
                        break; 
                    }
                }
            }
            std::set<char>::const_iterator b{ this->available_characters.cbegin() }; 
            std::advance(b, ind); 
            return *b; 
        }

        std::string operator()(const std::string &str) {
            std::string result{ str }; 
            for (char &e : result) {
                e = (*this)(e).value_or(e); 
                ++(*this); 
            }
            return result; 
        }
    }; 
}

#endif // ! DORAYAKI_ENIGMA_HPP_INCLUDED

