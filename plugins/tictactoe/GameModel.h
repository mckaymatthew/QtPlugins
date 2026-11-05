#pragma once

#include <QList>
#include <QObject>

// Tic-tac-toe rules and an optional computer opponent. Has no rendering dependencies so it can be
// unit tested headless; the QML board binds to its properties and calls play()/reset().
class GameModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<int> cells READ cells NOTIFY cellsChanged)
    Q_PROPERTY(Player currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(Player winner READ winner NOTIFY gameOverChanged)
    Q_PROPERTY(bool gameOver READ isGameOver NOTIFY gameOverChanged)
    Q_PROPERTY(bool draw READ isDraw NOTIFY gameOverChanged)
    Q_PROPERTY(QList<int> winningLine READ winningLine NOTIFY gameOverChanged)
    Q_PROPERTY(bool vsComputer READ vsComputer WRITE setVsComputer NOTIFY vsComputerChanged)
    Q_PROPERTY(int computerDelay READ computerDelay WRITE setComputerDelay NOTIFY computerDelayChanged)
    Q_PROPERTY(int xWins READ xWins NOTIFY scoreChanged)
    Q_PROPERTY(int oWins READ oWins NOTIFY scoreChanged)
    Q_PROPERTY(int draws READ draws NOTIFY scoreChanged)

public:
    enum Player { None = 0, X = 1, O = 2 };
    Q_ENUM(Player)

    static constexpr int CellCount = 9;

    explicit GameModel(QObject *parent = nullptr);

    QList<int> cells() const { return m_cells; }
    Player cellAt(int index) const;
    Player currentPlayer() const { return m_currentPlayer; }
    Player winner() const { return m_winner; }
    bool isGameOver() const { return m_winner != None || m_draw; }
    bool isDraw() const { return m_draw; }
    QList<int> winningLine() const { return m_winningLine; }

    // When enabled the human plays X and the computer replies as O.
    bool vsComputer() const { return m_vsComputer; }
    void setVsComputer(bool enabled);

    // Milliseconds before the computer replies; 0 replies synchronously inside play().
    int computerDelay() const { return m_computerDelay; }
    void setComputerDelay(int ms);

    int xWins() const { return m_xWins; }
    int oWins() const { return m_oWins; }
    int draws() const { return m_draws; }

    // Places the current player's mark. Returns false for illegal moves (occupied cell, out of
    // range, game over, or the computer's turn).
    Q_INVOKABLE bool play(int index);
    Q_INVOKABLE void reset();

    // Best move for the current player (minimax), or -1 if the game is over.
    Q_INVOKABLE int bestMove() const;

signals:
    void cellsChanged();
    void currentPlayerChanged();
    void gameOverChanged();
    void vsComputerChanged();
    void computerDelayChanged();
    void scoreChanged();
    void moveMade(int index, GameModel::Player player);

private:
    bool isComputerTurn() const { return m_vsComputer && m_currentPlayer == O; }
    void place(int index);
    void scheduleComputerMove();
    void computerMove();

    QList<int> m_cells;
    Player m_currentPlayer = X;
    Player m_winner = None;
    bool m_draw = false;
    QList<int> m_winningLine;
    bool m_vsComputer = false;
    int m_computerDelay = 400;
    int m_xWins = 0;
    int m_oWins = 0;
    int m_draws = 0;
    int m_generation = 0; // invalidates pending computer moves on reset()
};
