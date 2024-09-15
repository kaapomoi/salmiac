/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "board.h"

#include <bitset>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

namespace bbm {

Board::Board() noexcept : m_spawn_positions{}, m_board_state{} {}

Board::Board(std::string const& filename) noexcept
{
    /// SIZE_X,SIZE_Y
    /// <DATA>

    std::ifstream infile{filename};
    std::size_t w{0};
    std::size_t h{0};
    char delim{0};
    infile >> w >> delim >> h;
    // Ignore the first line.
    std::string first_line;
    std::getline(infile, first_line);

    m_board_state.resize(h, std::vector<Cell>(w));
    m_spawn_positions.resize(8);

    for (int y = 0; y < h; ++y) {
        std::string line;
        std::getline(infile, line);
        for (int x = 0; x < w; ++x) {
            char c = line.at(x);

            /// 8 player maximum
            if (c >= '1' && c <= '8') {
                m_spawn_positions.at(c - '1') = {x, y};
            }

            switch (c) {
            case '.':
                m_board_state[y][x].type = Cell::Type::empty;
                break;
            case 'W':
                m_board_state[y][x].type = Cell::Type::wall;
                break;
            case 'b':
                m_board_state[y][x].type = Cell::Type::box;
                break;

            default:
                m_board_state[y][x].type = Cell::Type::empty;
                break;
            }
        }
    }
}

Board::Board(std::size_t board_h, std::size_t board_w) noexcept
{
    m_board_state.resize(board_h, std::vector<Cell>(board_w));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);

    for (std::size_t i = 0; i < board_h; i++) {
        for (std::size_t j = 0; j < board_w; j++) {
            switch (dis(gen)) {
            case 0:
                m_board_state[i][j].type = Cell::Type::empty;
                break;
            case 1:
                m_board_state[i][j].type = Cell::Type::box;
                break;
            case 2:
                m_board_state[i][j].type = Cell::Type::wall;
                break;
            }
        }
    }
}

void Board::print_board() const noexcept
{
    for (const auto& row : m_board_state) {
        for (const auto& cell : row) {
            switch (cell.type) {
            case Cell::Type::empty:
                std::cout << ".";
                break;
            case Cell::Type::box:
                std::cout << "b";
                break;
            case Cell::Type::wall:
                std::cout << "X";
                break;
            }
        }
        std::cout << "\n";
    }
}

std::string Board::serialize_for_network() const noexcept
{
    std::string result{std::to_string(m_board_state.size()) + ","
                       + std::to_string(m_board_state.at(0).size()) + ";"};
    /// "15,15;<data>"
    /// data representation:
    /// 00 empty
    /// 01 box
    /// 10 wall
    /// 10000101 = bbew !!!

    for (const auto& row : m_board_state) {
        char current_byte{0};
        std::int32_t bit_count{0};

        for (auto const& cell : row) {
            current_byte |= ((static_cast<char>(cell.type) & 0b11) << bit_count);
            bit_count += 2;

            if (bit_count == 8) {
                //std::cout << "cb=" << std::bitset<8>(current_byte) << "\n";
                result.push_back(current_byte);
                bit_count = 0;
                current_byte = 0;
            }
        }

        /// Add last byte even if it isn't full.
        if (bit_count > 0) {
            //std::cout << "cba=" << std::bitset<8>(current_byte) << "\n";
            result.push_back(current_byte);
        }
    }

    //std::cout << result.size() << "\n";

    return result;
}


void Board::deserialize_from_network(std::string const& data) noexcept
{
    /// header = "15,15;<data>"
    /// data representation:
    /// 00 empty
    /// 01 box
    /// 10 wall
    m_board_state.clear();

    std::istringstream iss{data};
    std::size_t w{0};
    std::size_t h{0};
    char delim;

    iss >> w;
    iss >> delim;
    iss >> h;

    auto data_start = data.find(';') + 1;
    m_board_state.resize(h, std::vector<Cell>(w));

    for (std::size_t i = 0; i < h; i++) {
        for (std::size_t j = 0; j < w; j) {
            auto current_byte = data.at(data_start);
            for (int c = 0; c < 8; c += 2) {

                // Extract 2 bits from the current byte
                std::uint8_t value = (current_byte >> c) & 0b11;

                if (j >= w) {
                    break;
                }
                // Convert the 2-bit value to the enum class Type
                m_board_state[i][j++].type = static_cast<Cell::Type>(value);
            }
            data_start++;
        }
    }
}

std::optional<std::pair<std::size_t, std::size_t>> Board::size() const noexcept
{
    if (!m_board_state.empty()) {
        return {{m_board_state.at(0).size(), m_board_state.size()}};
    }
    else {
        return {};
    }
}

Cell::Type Board::at(std::size_t const x, std::size_t const y) const noexcept
{
    return m_board_state.at(y).at(x).type;
}

Cell& Board::at_mut(std::size_t const x, std::size_t const y) noexcept
{
    return m_board_state.at(y).at(x);
}

std::vector<std::pair<std::size_t, std::size_t>> Board::spawn_positions() noexcept
{
    return m_spawn_positions;
}

void Board::reset() noexcept
{
    m_board_state.clear();
    m_spawn_positions.clear();
}


} // namespace bbm
