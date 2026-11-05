#include "GameModel.h"

#include <QRandomGenerator>
#include <QTimer>

#include <algorithm>
#include <array>

namespace {

using Board = std::array<int, GameModel::CellCount>;

constexpr int Lines[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // columns
    {0, 4, 8}, {2, 4, 6},            // diagonals
};

// Returns the index into Lines of a completed line, or -1.
int findWinningLine(const Board &board)
{
    for (int i = 0; i < 8; ++i) {
        const int a = board[Lines[i][0]];
        if (a != GameModel::None && a == board[Lines[i][1]] && a == board[Lines[i][2]])
            return i;
    }
    return -1;
}

bool isFull(const Board &board)
{
    for (int cell : board) {
        if (cell == GameModel::None)
            return false;
    }
    return true;
}

int other(int player)
{
    return player == GameModel::X ? GameModel::O : GameModel::X;
}

// Score from `me`'s perspective with `toMove` to play: +10 win, -10 loss, 0 draw. Depth makes
// faster wins and slower losses preferable.
int minimax(Board &board, int toMove, int me, int depth)
{
    const int line = findWinningLine(board);
    if (line >= 0)
        return board[Lines[line][0]] == me ? 10 - depth : depth - 10;
    if (isFull(board))
        return 0;

    int best = toMove == me ? -100 : 100;
    for (int i = 0; i < GameModel::CellCount; ++i) {
        if (board[i] != GameModel::None)
            continue;
        board[i] = toMove;
        const int score = minimax(board, other(toMove), me, depth + 1);
        board[i] = GameModel::None;
        best = toMove == me ? qMax(best, score) : qMin(best, score);
    }
    return best;
}

} // namespace

GameModel::GameModel(QObject *parent)
    : QObject(parent)
    , m_cells(CellCount, None)
{
}

GameModel::Player GameModel::cellAt(int index) const
{
    return index >= 0 && index < CellCount ? Player(m_cells[index]) : None;
}

void GameModel::setVsComputer(bool enabled)
{
    if (m_vsComputer == enabled)
        return;
    m_vsComputer = enabled;
    emit vsComputerChanged();
    if (isComputerTurn() && !isGameOver())
        scheduleComputerMove();
}

void GameModel::setComputerDelay(int ms)
{
    ms = qMax(0, ms);
    if (m_computerDelay == ms)
        return;
    m_computerDelay = ms;
    emit computerDelayChanged();
}

bool GameModel::play(int index)
{
    if (index < 0 || index >= CellCount || m_cells[index] != None || isGameOver()
        || isComputerTurn()) {
        return false;
    }
    place(index);
    if (isComputerTurn() && !isGameOver())
        scheduleComputerMove();
    return true;
}

void GameModel::reset()
{
    ++m_generation;
    const bool wasOver = isGameOver();
    m_cells.fill(None);
    m_winner = None;
    m_draw = false;
    m_winningLine.clear();
    emit cellsChanged();
    if (wasOver)
        emit gameOverChanged();
    if (m_currentPlayer != X) {
        m_currentPlayer = X;
        emit currentPlayerChanged();
    }
}

int GameModel::bestMove() const
{
    if (isGameOver())
        return -1;

    Board board;
    std::copy(m_cells.cbegin(), m_cells.cend(), board.begin());
    const int me = m_currentPlayer;

    int bestScore = -100;
    QList<int> bestMoves;
    for (int i = 0; i < CellCount; ++i) {
        if (board[i] != None)
            continue;
        board[i] = me;
        const int score = minimax(board, other(me), me, 1);
        board[i] = None;
        if (score > bestScore) {
            bestScore = score;
            bestMoves = {i};
        } else if (score == bestScore) {
            bestMoves.append(i);
        }
    }
    // Pick randomly among equally good moves so games vary.
    return bestMoves.value(QRandomGenerator::global()->bounded(int(bestMoves.size())), -1);
}

void GameModel::place(int index)
{
    m_cells[index] = m_currentPlayer;
    const Player mover = m_currentPlayer;
    emit cellsChanged();
    emit moveMade(index, mover);

    Board board;
    std::copy(m_cells.cbegin(), m_cells.cend(), board.begin());
    const int line = findWinningLine(board);
    if (line >= 0) {
        m_winner = mover;
        m_winningLine = {Lines[line][0], Lines[line][1], Lines[line][2]};
        ++(mover == X ? m_xWins : m_oWins);
        emit gameOverChanged();
        emit scoreChanged();
        return;
    }
    if (isFull(board)) {
        m_draw = true;
        ++m_draws;
        emit gameOverChanged();
        emit scoreChanged();
        return;
    }
    m_currentPlayer = Player(other(mover));
    emit currentPlayerChanged();
}

void GameModel::scheduleComputerMove()
{
    if (m_computerDelay == 0) {
        computerMove();
        return;
    }
    const int generation = m_generation;
    QTimer::singleShot(m_computerDelay, this, [this, generation] {
        if (generation == m_generation)
            computerMove();
    });
}

void GameModel::computerMove()
{
    if (!isComputerTurn() || isGameOver())
        return;
    const int index = bestMove();
    if (index >= 0)
        place(index);
}
