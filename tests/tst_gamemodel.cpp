#include "GameModel.h"

#include <QSignalSpy>
#include <QTest>

#include <functional>

class TestGameModel : public QObject
{
    Q_OBJECT

private:
    static void playAll(GameModel &game, const QList<int> &moves)
    {
        for (int index : moves)
            QVERIFY2(game.play(index), qPrintable(QStringLiteral("move %1").arg(index)));
    }

private slots:
    void initialState()
    {
        GameModel game;
        QCOMPARE(game.cells(), QList<int>(9, GameModel::None));
        QCOMPARE(game.currentPlayer(), GameModel::X);
        QCOMPARE(game.winner(), GameModel::None);
        QVERIFY(!game.isGameOver());
        QVERIFY(!game.isDraw());
        QVERIFY(game.winningLine().isEmpty());
    }

    void alternatesPlayers()
    {
        GameModel game;
        QSignalSpy moves(&game, &GameModel::moveMade);
        QVERIFY(game.play(4));
        QCOMPARE(game.cellAt(4), GameModel::X);
        QCOMPARE(game.currentPlayer(), GameModel::O);
        QVERIFY(game.play(0));
        QCOMPARE(game.cellAt(0), GameModel::O);
        QCOMPARE(game.currentPlayer(), GameModel::X);
        QCOMPARE(moves.count(), 2);
        QCOMPARE(moves.at(1).at(0).toInt(), 0);
        QCOMPARE(moves.at(1).at(1).value<GameModel::Player>(), GameModel::O);
    }

    void rejectsIllegalMoves()
    {
        GameModel game;
        QVERIFY(!game.play(-1));
        QVERIFY(!game.play(9));
        QVERIFY(game.play(0));
        QVERIFY(!game.play(0)); // occupied
        QCOMPARE(game.currentPlayer(), GameModel::O);
    }

    void detectsWins_data()
    {
        QTest::addColumn<QList<int>>("xCells");
        QTest::addColumn<QList<int>>("oCells");
        // O's filler moves never complete a line of their own.
        QTest::newRow("row 0") << QList<int>{0, 1, 2} << QList<int>{3, 4};
        QTest::newRow("row 1") << QList<int>{3, 4, 5} << QList<int>{0, 1};
        QTest::newRow("row 2") << QList<int>{6, 7, 8} << QList<int>{0, 1};
        QTest::newRow("col 0") << QList<int>{0, 3, 6} << QList<int>{1, 2};
        QTest::newRow("col 1") << QList<int>{1, 4, 7} << QList<int>{0, 2};
        QTest::newRow("col 2") << QList<int>{2, 5, 8} << QList<int>{0, 1};
        QTest::newRow("diag") << QList<int>{0, 4, 8} << QList<int>{1, 2};
        QTest::newRow("anti-diag") << QList<int>{2, 4, 6} << QList<int>{0, 1};
    }

    void detectsWins()
    {
        QFETCH(QList<int>, xCells);
        QFETCH(QList<int>, oCells);

        GameModel game;
        QSignalSpy over(&game, &GameModel::gameOverChanged);
        playAll(game, {xCells[0], oCells[0], xCells[1], oCells[1], xCells[2]});

        QVERIFY(game.isGameOver());
        QCOMPARE(game.winner(), GameModel::X);
        QVERIFY(!game.isDraw());
        QCOMPARE(game.winningLine(), xCells);
        QCOMPARE(game.xWins(), 1);
        QCOMPARE(over.count(), 1);
        QVERIFY(!game.play(oCells.contains(8) ? 7 : 8)); // no moves after game over
    }

    void oCanWin()
    {
        GameModel game;
        playAll(game, {0, 3, 1, 4, 8, 5});
        QCOMPARE(game.winner(), GameModel::O);
        QCOMPARE(game.winningLine(), (QList<int>{3, 4, 5}));
        QCOMPARE(game.oWins(), 1);
    }

    void detectsDraw()
    {
        GameModel game;
        // X O X / X O O / O X X
        playAll(game, {0, 1, 2, 4, 3, 5, 7, 6, 8});
        QVERIFY(game.isGameOver());
        QVERIFY(game.isDraw());
        QCOMPARE(game.winner(), GameModel::None);
        QCOMPARE(game.draws(), 1);
    }

    void winOnLastCellIsNotADraw()
    {
        GameModel game;
        // X O O / O X X / X O X: the ninth move completes the 0-4-8 diagonal.
        playAll(game, {0, 1, 4, 2, 5, 3, 6, 7, 8});
        QCOMPARE(game.winner(), GameModel::X);
        QVERIFY(!game.isDraw());
        QCOMPARE(game.draws(), 0);
    }

    void resetClearsBoardButKeepsScore()
    {
        GameModel game;
        playAll(game, {0, 3, 1, 4, 2});
        QVERIFY(game.isGameOver());
        game.reset();
        QCOMPARE(game.cells(), QList<int>(9, GameModel::None));
        QCOMPARE(game.currentPlayer(), GameModel::X);
        QVERIFY(!game.isGameOver());
        QVERIFY(game.winningLine().isEmpty());
        QCOMPARE(game.xWins(), 1);
        QVERIFY(game.play(0));
    }

    void bestMoveTakesWin()
    {
        GameModel game;
        playAll(game, {0, 3, 1, 4}); // X to move, can win at 2
        QCOMPARE(game.bestMove(), 2);
    }

    void bestMoveBlocks()
    {
        GameModel game;
        playAll(game, {0, 4, 1}); // O to move, must block at 2
        QCOMPARE(game.bestMove(), 2);
    }

    void computerReplies()
    {
        GameModel game;
        game.setComputerDelay(0);
        game.setVsComputer(true);
        QVERIFY(game.play(0));
        QCOMPARE(game.cells().count(GameModel::O), 1);
        QCOMPARE(game.currentPlayer(), GameModel::X);
    }

    void humanCannotMoveForComputer()
    {
        GameModel game;
        game.setComputerDelay(50);
        game.setVsComputer(true);
        QVERIFY(game.play(4));
        QVERIFY(!game.play(0)); // computer is still thinking
        QTRY_COMPARE(game.currentPlayer(), GameModel::X);
    }

    void resetCancelsPendingComputerMove()
    {
        GameModel game;
        game.setComputerDelay(20);
        game.setVsComputer(true);
        QVERIFY(game.play(4));
        game.reset();
        QTest::qWait(60);
        QCOMPARE(game.cells(), QList<int>(9, GameModel::None));
    }

    void computerNeverLoses()
    {
        // Exhaustively try every human (X) strategy against bestMove() playing O. The computer
        // picks randomly among equally good moves, so replay the full recorded sequence.
        int games = 0;
        std::function<void(const QList<int> &)> explore = [&](const QList<int> &history) {
            for (int i = 0; i < GameModel::CellCount; ++i) {
                GameModel game;
                for (int index : history)
                    game.play(index);
                if (!game.play(i))
                    continue;
                QList<int> next = history + QList<int>{i};
                if (!game.isGameOver()) {
                    const int reply = game.bestMove();
                    QVERIFY(game.play(reply));
                    next.append(reply);
                }
                if (game.isGameOver()) {
                    QVERIFY2(game.winner() != GameModel::X, "computer lost");
                    ++games;
                } else {
                    explore(next);
                }
            }
        };
        explore({});
        QVERIFY(games > 0);
        qInfo("explored %d games", games);
    }
};

QTEST_GUILESS_MAIN(TestGameModel)
#include "tst_gamemodel.moc"
