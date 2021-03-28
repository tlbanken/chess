/*
 * chess.cpp
 *
 * Travis Banken
 *
 * Simple implementation of chess, using a form of the MVC design pattern.
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <cassert>

#define SPACE_SIZE 100.0
#define WIN_W (8 * SPACE_SIZE)
#define WIN_H (8 * SPACE_SIZE)

#define SPACE_WHITE sf::Color(134, 149, 173)
#define SPACE_BLACK sf::Color(51, 58, 69)

struct BoardCoord {
    int x = 0;
    int y = 0;
    BoardCoord(int x, int y)
        : x(x), y(y) {}

    friend std::ostream& operator<<(std::ostream& os, const BoardCoord& bc)
    {
        os << "(" << bc.x << ", " << bc.y << ")";
        return os;
    }

    bool operator==(const BoardCoord& other)
    {
        return x == other.x && y == other.y;
    }
};

struct SpriteDim {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    SpriteDim(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {}
};

enum class Player {
    White,
    Black
};
std::string PlayerToString(Player p)
{
    return p == Player::White ? "White" : "Black";
}

enum class PieceType {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

//=====================================
// Game Pieces
//=====================================
class Piece {
public:
    Piece(Player p)
    {
        m_player = p;
    }

    Player GetPlayer()
    {
        return m_player;
    }

    virtual bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to) = 0;
    virtual SpriteDim GetSpriteDim() = 0;
    virtual enum PieceType PieceType() = 0;

protected:
    Player m_player;
};

class King : public Piece {
public:
    King(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        // can only move 1 space in any direction
        if (abs(fx - tx) > 1 || abs(fy - ty) > 1) {
            return false;
        }

        return true;
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {860, 0, 300, 273};
        } else {
            return {860, 340, 300, 314};
        }
    }

    enum PieceType PieceType() {return PieceType::King;}
private:
};

class Queen : public Piece {
public:
    Queen(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        if (to == from) return false;
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        if (abs(fx - tx) == abs(fy - ty)) { // diagonal
            // walk the diagonal to see if any piece blocks
            int xstep = fx < tx ? 1 : -1;
            int ystep = fy < ty ? 1 : -1;
            int ix = fx + xstep;
            int iy = fy + ystep;
            while (ix != tx) {
                auto piece = board.at(iy * 8 + ix);
                if (piece != nullptr) {
                    // blocked
                    return false;
                }

                // step
                ix += xstep;
                iy += ystep;
            }
            return true;
        } else if ((fx == tx && fy != ty) || (fy == ty && fx != tx)) { // horz/vert
            // walk either x or y direction to see if blocked by any piece
            auto [start, end] = (tx != fx) ? BoardCoord{fx, tx} : BoardCoord{fy, ty};
            int step = start < end ? 1 : -1; // choose step dir
            for (int i = start + step; i != end; i += step) {
                auto piece = (tx != fx) ? board.at(ty * 8 + i) : board.at(i * 8 + tx);
                if (piece != nullptr) {
                    // blocked
                    return false;
                }
            }
            return true;
        }

        return false;
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {557, 0, 298, 292};
        } else {
            return {557, 351, 298, 292};
        }
    }

    enum PieceType PieceType() {return PieceType::Queen;}
private:
};

class Rook : public Piece {
public:
    Rook(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        if (to == from) return false;
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        // can't move diagonally
        if (tx != fx && ty != fy) {
            return false;
        }

        // now walk either x or y direction to see if blocked by any piece
        auto [start, end] = (tx != fx) ? BoardCoord{fx, tx} : BoardCoord{fy, ty};
        int step = start < end ? 1 : -1; // choose step dir
        for (int i = start + step; i != end; i += step) {
            auto piece = (tx != fx) ? board.at(ty * 8 + i) : board.at(i * 8 + tx);
            if (piece != nullptr) {
                // blocked
                return false;
            }
        }

        return true;
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {0, 0, 250, 280};
        } else {
            return {0, 368, 250, 280};
        }
    }

    enum PieceType PieceType() {return PieceType::Rook;}
private:
};

class Bishop : public Piece {
public:
    Bishop(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        if (to == from) return false;
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        // can't vert/horz
        if (tx == fx || ty == fy) {
            return false;
        }

        // check if possible to move there diagonally
        if (abs(fx - tx) != abs(fy - ty)) {
            return false;
        }

        // walk the diagonal to see if any piece blocks
        int xstep = fx < tx ? 1 : -1;
        int ystep = fy < ty ? 1 : -1;
        int ix = fx + xstep;
        int iy = fy + ystep;
        while (ix != tx) {
            auto piece = board.at(iy * 8 + ix);
            if (piece != nullptr) {
                // blocked
                return false;
            }

            // step
            ix += xstep;
            iy += ystep;
        }

        return true;
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {246, 0, 305, 300};
        } else {
            return {246, 354, 305, 300};
        }
    }

    enum PieceType PieceType() {return PieceType::Bishop;}
private:
};

class Knight : public Piece {
public:
    Knight(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        return (abs(tx - fx) == 1 && abs(ty - fy) == 2)
            || (abs(tx - fx) == 2 && abs(ty - fy) == 1);
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {1164, 0, 280, 267};
        } else {
            return {1164, 370, 280, 267};
        }
    }

    enum PieceType PieceType() {return PieceType::Knight;}
private:
};

class Pawn : public Piece {
public:
    Pawn(Player p)
        : Piece(p) {}

    bool IsValidMove(const std::vector<std::shared_ptr<Piece>>& board, BoardCoord from, BoardCoord to)
    {
        auto& [tx, ty] = to;
        auto& [fx, fy] = from;
        // make sure dest is in-bounds
        if (tx >= 8 || tx < 0 || ty >= 8 || ty < 0) {
            return false;
        }

        // valid moves are forward one space, or diag attacks
        bool valid_y = m_player == Player::White ? fy - ty == 1 : ty - fy == 1;
        bool valid_x = abs(tx - fx) <= 1;
        if (valid_y && valid_x) {
            // check diag
            auto piece = board[ty * 8 + tx];
            if (abs(tx - fx) == 1) {
                return piece != nullptr; // check if attacking
            }
            return piece == nullptr; // can't attack forward
        }

        return false;
    }

    SpriteDim GetSpriteDim()
    {
        if (m_player == Player::Black) {
            return {1490, 0, 230, 270};
        } else {
            return {1490, 376, 230, 270};
        }
    }

    enum PieceType PieceType() {return PieceType::Pawn;}
private:
};


//=====================================
// Board Model
//=====================================
class BoardModel {
public:
    BoardModel()
    {
        m_board.resize(64); // 64 spaces
        // first fill empty board
        for (auto& p : m_board) {
            p = nullptr;
        }
        // now fill pieces
        // black
        m_board[0 * 8 + 0] = static_cast<std::shared_ptr<Piece>>(new Rook(Player::Black));
        m_board[0 * 8 + 1] = static_cast<std::shared_ptr<Piece>>(new Knight(Player::Black));
        m_board[0 * 8 + 2] = static_cast<std::shared_ptr<Piece>>(new Bishop(Player::Black));
        m_board[0 * 8 + 3] = static_cast<std::shared_ptr<Piece>>(new Queen(Player::Black));
        m_board[0 * 8 + 4] = static_cast<std::shared_ptr<Piece>>(new King(Player::Black));
        m_board[0 * 8 + 5] = static_cast<std::shared_ptr<Piece>>(new Bishop(Player::Black));
        m_board[0 * 8 + 6] = static_cast<std::shared_ptr<Piece>>(new Knight(Player::Black));
        m_board[0 * 8 + 7] = static_cast<std::shared_ptr<Piece>>(new Rook(Player::Black));
        m_board[1 * 8 + 0] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 1] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 2] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 3] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 4] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 5] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 6] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));
        m_board[1 * 8 + 7] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::Black));

        // White
        m_board[7 * 8 + 0] = static_cast<std::shared_ptr<Piece>>(new Rook(Player::White));
        m_board[7 * 8 + 1] = static_cast<std::shared_ptr<Piece>>(new Knight(Player::White));
        m_board[7 * 8 + 2] = static_cast<std::shared_ptr<Piece>>(new Bishop(Player::White));
        m_board[7 * 8 + 3] = static_cast<std::shared_ptr<Piece>>(new Queen(Player::White));
        m_board[7 * 8 + 4] = static_cast<std::shared_ptr<Piece>>(new King(Player::White));
        m_board[7 * 8 + 5] = static_cast<std::shared_ptr<Piece>>(new Bishop(Player::White));
        m_board[7 * 8 + 6] = static_cast<std::shared_ptr<Piece>>(new Knight(Player::White));
        m_board[7 * 8 + 7] = static_cast<std::shared_ptr<Piece>>(new Rook(Player::White));
        m_board[6 * 8 + 0] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 1] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 2] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 3] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 4] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 5] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 6] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
        m_board[6 * 8 + 7] = static_cast<std::shared_ptr<Piece>>(new Pawn(Player::White));
    }

    bool MovePiece(Player p, BoardCoord from, BoardCoord to)
    {
        auto attacker_piece = GetPieceAt(from);
        if (attacker_piece == nullptr) {
            return false;
        }
        auto defender_piece = GetPieceAt(to);

        bool valid_move = attacker_piece->IsValidMove(m_board, from, to);
        if (valid_move && (defender_piece == nullptr || defender_piece->GetPlayer() != p)) {
            if (defender_piece != nullptr && defender_piece->PieceType() == PieceType::King) {
                m_game_over = true;
            }
            m_board[(to.y * 8) + to.x] = attacker_piece;
            m_board[(from.y * 8) + from.x] = nullptr;
            return true;
        }
        return false;
    }

    std::shared_ptr<Piece> GetPieceAt(BoardCoord c) const
    {
        auto [x, y] = c;
        int index = (y * 8) + x;
        assert(index < m_board.size());
        return m_board[index];
    }

    bool SpaceSelected() const
    {
        return m_move_started;
    }

    BoardCoord GetSpaceSelected() const
    {
        return m_space_selected;
    }

    void SelectSpace(BoardCoord coord)
    {
        if (GameOver()) {
            return;
        }

        if (m_move_started) {
            bool valid_move = MovePiece(m_cur_player, m_space_selected, coord);
            m_move_started = false;
            if (valid_move) {
                m_cur_player = m_cur_player == Player::White ? Player::Black : Player::White;
            }
            return;
        }
        // check if clicked on empty space
        if (GetPieceAt(coord) == nullptr) {
            return;
        }

        // check if selecting your piece
        if (GetPieceAt(coord)->GetPlayer() != m_cur_player) {
            return;
        }

        m_space_selected = coord;
        m_move_started = true;
    }

    Player GetCurPlayer()
    {
        return m_cur_player;
    }

    bool GameOver()
    {
        return m_game_over;
    }

private:
    std::vector<std::shared_ptr<Piece>> m_board;
    bool m_move_started = false;
    BoardCoord m_space_selected{0,0};
    bool m_game_over = false;
    Player m_cur_player = Player::White;
};

//=====================================
// Board Controller
//=====================================
class BoardController {
public:
    BoardController(std::shared_ptr<BoardModel> model)
        : m_board(model) {}

    void OnMouseClicked(float x, float y)
    {
        BoardCoord space_coord = {static_cast<int>(x / SPACE_SIZE), static_cast<int>(y / SPACE_SIZE)};
        m_board->SelectSpace(space_coord);
    }

private:
    std::shared_ptr<BoardModel> m_board;
};

//=====================================
// Board View
//=====================================
class BoardView {
public:
    BoardView(std::shared_ptr<BoardModel> board)
        : m_board(board)
    {
        bool success = m_pieces_tex.loadFromFile("pieces.png");
        if (!success) {
            throw std::runtime_error("Failed to load texture from pieces.png");
        }
    }

    void Draw(sf::RenderWindow& window)
    {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                sf::RectangleShape space(sf::Vector2f(SPACE_SIZE, SPACE_SIZE));
                space.setPosition(col * SPACE_SIZE, row * SPACE_SIZE);

                // alternate white-black spaces
                sf::Color color;
                if (row % 2 == 0) {
                    color = col % 2 == 0 ? SPACE_WHITE : SPACE_BLACK;
                } else {
                    color = col % 2 != 0 ? SPACE_WHITE : SPACE_BLACK;
                }
                space.setOutlineThickness(4);
                space.setOutlineColor(sf::Color(0, 0, 0));
                space.setFillColor(color);
                window.draw(space);

                // draw piece
                auto piece = m_board->GetPieceAt(BoardCoord{col, row});
                if (piece != nullptr) {
                    sf::Sprite piece_sprite;
                    piece_sprite.setTexture(m_pieces_tex);
                    auto [x, y, w, h] = piece->GetSpriteDim();
                    piece_sprite.setTextureRect(sf::IntRect(x, y, w, h));
                    piece_sprite.setPosition(col * SPACE_SIZE, row * SPACE_SIZE);
                    piece_sprite.scale(SPACE_SIZE / w, SPACE_SIZE / h);
                    window.draw(piece_sprite);
                }
            }
        }

        if (m_board->SpaceSelected()) {
            auto [x, y] = m_board->GetSpaceSelected();
            // highlight currently selected piece
            sf::RectangleShape highlight(sf::Vector2f(SPACE_SIZE, SPACE_SIZE));
            highlight.setOutlineColor(sf::Color(255, 255, 0));
            highlight.setOutlineThickness(4);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setPosition(x * SPACE_SIZE, y * SPACE_SIZE);
            window.draw(highlight);
        }

    }
private:
    std::shared_ptr<BoardModel> m_board;
    sf::Texture m_pieces_tex;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Chess");

    // setup mvc
    auto model = std::shared_ptr<BoardModel>(new BoardModel());
    auto controller = std::shared_ptr<BoardController>(new BoardController(model));
    auto view = std::shared_ptr<BoardView>(new BoardView(model));


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                auto [x, y] = sf::Mouse::getPosition(window);
                controller->OnMouseClicked(x, y);
            }
        }

        auto cur_player = model->GetCurPlayer();
        if (!model->GameOver()) {
            window.setTitle("Chess: " + PlayerToString(cur_player) + "'s Move");
        } else {
            std::string winner = cur_player == Player::White ? "Black" : "White";
            window.setTitle("Chess: Game Finished - " + winner + " Wins!");
        }
        window.clear();
        view->Draw(window);
        window.display();

    }

    return 0;
}
