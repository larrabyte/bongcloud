#pragma once

#include "extras.hpp"
#include "pieces.hpp"

#include <centurion.hpp>
#include <optional>
#include <cstddef>
#include <vector>

namespace bcl {
    // A square is just an optional piece.
    using square = std::optional<piece>;

    template<typename T>
    struct pair {
        std::array<T, 2> underlying;

        constexpr T& white(void) const noexcept { return underlying[0]; }
        constexpr T& black(void) const noexcept { return underlying[1]; }

        constexpr T& operator[](const piece::color c) noexcept {
            return underlying[ext::to_underlying(c)];
        }

        constexpr const T& operator[](const piece::color c) const noexcept {
            return underlying[ext::to_underlying(c)];
        }
    };

    struct rights {
        bool kingside;
        bool queenside;
    };

    struct move {
        std::size_t from;
        std::size_t to;
    };

    struct index {
        std::size_t rank;
        std::size_t file;
    };

    struct capture {
        std::size_t index;
        bcl::piece piece;
    };

    struct record {
        piece::color color;
        bcl::move move;
        std::size_t trivials;
        bcl::pair<bcl::rights> rights;
        std::optional<bcl::move> castle;
        std::optional<bcl::capture> capture;
        std::optional<piece> promotion;
    };

    class board {
        public:
            board(const std::size_t l, const bool a) noexcept : length {l}, m_internal {l * l}, m_anarchy {a} {}

            // Attempts to move a piece from one square to another.
            bool move(const std::size_t, const std::size_t) noexcept;

            // Generates a list of all legal moves for the current player.
            std::vector<bcl::move> moves(void) noexcept;

            // An algorithm that counts possible positions recursively.
            std::size_t positions(const std::size_t) noexcept;

            // Returns whether the player is currently in check.
            bool check(void) const noexcept;

            // Returns whether the player has been checkmated.
            bool checkmate(void) noexcept;

            // Returns whether the player has been stalemated.
            bool stalemate(void) noexcept;

            // Prints out the current board state to stdout.
            void print(void) const noexcept;

            // Overwrites the current board state using a FEN string.
            void load(const std::string_view);

            // Undoes the last move.
            void undo(void) noexcept;

            // Returns a constant reference to the board's history array.
            const std::vector<record>& history(void) const noexcept {
                return m_history;
            }

            // Returns the last move made (may be std::nullopt).
            std::optional<bcl::move> latest(void) const noexcept {
                return (m_history.size() > 0) ? std::optional(m_history.back().move) : std::nullopt;
            }

            // Returns the color of the player whose turn it is to move.
            piece::color color(void) const noexcept {
                return m_color;
            }

            // Provides support for range-based for loops.
            std::vector<square>::iterator begin(void) noexcept {
                return m_internal.begin();
            }

            std::vector<square>::const_iterator begin(void) const noexcept {
                return m_internal.cbegin();
            }

            std::vector<square>::iterator end(void) noexcept {
                return m_internal.end();
            }

            std::vector<square>::const_iterator end(void) const noexcept {
                return m_internal.cend();
            }

            // Allow the use of array indexing syntax to access board squares.
            square& operator[] (const std::size_t i) noexcept {
                return m_internal[i];
            }

            const square& operator[] (const std::size_t i) const noexcept {
                return m_internal[i];
            }

            // The length of the board.
            const std::size_t length;

        private:
            // Returns the type of move (if pseudolegal) based on piece movement rules.
            std::optional<piece::move> pseudolegal(const std::size_t, const std::size_t) const noexcept;

            // The board's internal representation.
            std::vector<square> m_internal;

            // An array of previously made moves.
            std::vector<record> m_history;

            // Castling rights for each player.
            pair<rights> m_rights;

            // Determines whether any move is legal.
            bool m_anarchy;

            // Whose turn it is to move.
            piece::color m_color = piece::color::white;

            // The number of trivial half-moves made.
            std::size_t m_trivials = 0;
    };

    namespace constants {
        // The number of trivial half-moves until a forced draw.
        constexpr std::size_t trivial_force_draw = 100;

        // The average position has about 40 legal moves.
        constexpr std::size_t move_buffer_reserve = 40;
    }
}
