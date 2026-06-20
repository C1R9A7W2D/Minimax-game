#include <iostream>
#include <vector>
#include <sstream>
#include <limits>
#include <string>
#include <cstdlib>
#include <memory>

using namespace std;

const int ROWS = 6;
const int COLS = 7;
const int INF = 1001;
int nodeCount = 0;

class GameState {
public:
    vector<vector<int>> board;
    int currentPlayer;
    int lastMoveCol;
    int lastMoveRow;

    GameState() : currentPlayer(0), lastMoveCol(-1), lastMoveRow(-1) {
        board.resize(ROWS, vector<int>(COLS, -1));
    }

    GameState(const GameState& other) {
        board = other.board;
        currentPlayer = other.currentPlayer;
        lastMoveCol = other.lastMoveCol;
        lastMoveRow = other.lastMoveRow;
    }

    bool makeMove(int col) {
        if (col < 0 || col >= COLS) return false;

        for (int row = ROWS - 1; row >= 0; row--) {
            if (board[row][col] == -1) {
                board[row][col] = currentPlayer;
                lastMoveCol = col;
                lastMoveRow = row;
                currentPlayer = 1 - currentPlayer;
                return true;
            }
        }
        return false;
    }

    bool isValidMove(int col) const {
        return col >= 0 && col < COLS && board[0][col] == -1;
    }

    vector<int> getValidMoves() const {
        vector<int> moves;
        for (int col = 0; col < COLS; col++) {
            if (isValidMove(col)) {
                moves.push_back(col);
            }
        }
        return moves;
    }

    int checkWinner() const {
        if (lastMoveCol == -1) return -1;

        int player = board[lastMoveRow][lastMoveCol];

        // Проверка горизонталей
        int count = 1;
        for (int c = lastMoveCol - 1; c >= 0 && board[lastMoveRow][c] == player; c--) count++;
        for (int c = lastMoveCol + 1; c < COLS && board[lastMoveRow][c] == player; c++) count++;
        if (count >= 4) return player;

        // Проверка вертикалей
        count = 1;
        for (int r = lastMoveRow - 1; r >= 0 && board[r][lastMoveCol] == player; r--) count++;
        for (int r = lastMoveRow + 1; r < ROWS && board[r][lastMoveCol] == player; r++) count++;
        if (count >= 4) return player;

        // Проверка диагоналей (нисходящих)
        count = 1;
        for (int r = lastMoveRow - 1, c = lastMoveCol - 1; r >= 0 && c >= 0 && board[r][c] == player; r--, c--) count++;
        for (int r = lastMoveRow + 1, c = lastMoveCol + 1; r < ROWS && c < COLS && board[r][c] == player; r++, c++) count++;
        if (count >= 4) return player;

        // Проверка диагоналей (восходящих)
        count = 1;
        for (int r = lastMoveRow + 1, c = lastMoveCol - 1; r < ROWS && c >= 0 && board[r][c] == player; r++, c--) count++;
        for (int r = lastMoveRow - 1, c = lastMoveCol + 1; r >= 0 && c < COLS && board[r][c] == player; r--, c++) count++;
        if (count >= 4) return player;

        // Проверка на ничью
        bool isFull = true;
        for (int col = 0; col < COLS; col++) {
            if (board[0][col] == -1) {
                isFull = false;
                break;
            }
        }
        if (isFull) return 2; // Ничья

        return -1; // Игра продолжается
    }

    bool isTerminal() const {
        return checkWinner() != -1;
    }

    void printBoard() const {
        cout << "  0 1 2 3 4 5 6" << endl;
        for (int i = 0; i < ROWS; i++) {
            cout << "| ";
            for (int j = 0; j < COLS; j++) {
                if (board[i][j] == -1) {
                    cout << ". ";
                }
                else if (board[i][j] == 0) {
                    cout << "X ";
                }
                else {
                    cout << "O ";
                }
            }
            cout << "|" << endl;
        }
        cout << "+---------------+" << endl;
    }
};

class GameTree {
public:
    GameState state;
    int value;
    vector<unique_ptr<GameTree>> children;
    int move; // Ход, который привел к этому состоянию

    GameTree(const GameState& gameState, int move = -1) : state(gameState), value(0), move(move) {}

    void addChild(unique_ptr<GameTree> child) {
        children.push_back(std::move(child));
    }

    int getValue() const {
        return value;
    }

    const vector<unique_ptr<GameTree>>& getChildren() const {
        return children;
    }

    // Рекурсивное построение дерева игры до заданной глубины
    void buildTree(int depth, int maxDepth) {
        if (depth >= maxDepth || state.isTerminal()) {
            value = evaluateState();
            return;
        }

        vector<int> moves = state.getValidMoves();
        for (int move : moves) {
            GameState newState = state;
            if (newState.makeMove(move)) {
                auto child = make_unique<GameTree>(newState, move);
                child->buildTree(depth + 1, maxDepth);
                addChild(std::move(child));
            }
        }
    }

private:
    int evaluateState() const {
        int winner = state.checkWinner();
        if (winner == 0) return 1000; // Победа игрока 0
        if (winner == 1) return -1000; // Победа игрока 1
        if (winner == 2) return 0; // Ничья

        // Эвристическая оценка позиции
        int score = 0;

        // Оценка центральных колонок
        for (int row = 0; row < ROWS; row++) {
            if (state.board[row][3] == 0) score += 3;
            if (state.board[row][3] == 1) score -= 3;
        }

        // Оценка потенциальных линий
        score += evaluateLines(0) - evaluateLines(1);

        return score;
    }

    int evaluateLines(int player) const {
        int score = 0;

        // Проверяем все возможные линии по 4 клетки
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS - 3; col++) {
                score += evaluateLine(player, row, col, 0, 1); // Горизонталь
            }
        }

        for (int row = 0; row < ROWS - 3; row++) {
            for (int col = 0; col < COLS; col++) {
                score += evaluateLine(player, row, col, 1, 0); // Вертикаль
            }
        }

        for (int row = 0; row < ROWS - 3; row++) {
            for (int col = 0; col < COLS - 3; col++) {
                score += evaluateLine(player, row, col, 1, 1); // Диагональ \'
            }
        }

        for (int row = 3; row < ROWS; row++) {
            for (int col = 0; col < COLS - 3; col++) {
                score += evaluateLine(player, row, col, -1, 1); // Диагональ /
            }
        }

        return score;
    }

        int evaluateLine(int player, int startRow, int startCol, int deltaRow, int deltaCol) const {
            int playerCount = 0;
            int opponentCount = 0;
            int emptyCount = 0;

            for (int i = 0; i < 4; i++) {
                int row = startRow + i * deltaRow;
                int col = startCol + i * deltaCol;
                int cell = state.board[row][col];

                if (cell == player) playerCount++;
                else if (cell == (1 - player)) opponentCount++;
                else emptyCount++;
            }

            // Оценка на основе количества фишек в линии
            if (opponentCount > 0 && playerCount > 0) return 0; // Смешанная линия

            if (playerCount == 4) return 100;
            if (playerCount == 3 && emptyCount == 1) return 10;
            if (playerCount == 2 && emptyCount == 2) return 3;

            if (opponentCount == 4) return -100;
            if (opponentCount == 3 && emptyCount == 1) return -8;
            if (opponentCount == 2 && emptyCount == 2) return -2;

            return 0;
        }
    };

    int minimax(GameTree* node, int depth, int alpha, int beta, bool maximizingPlayer) {
        nodeCount++;

        if (node->getChildren().empty()) {
            return node->getValue();
        }

        if (maximizingPlayer) {
            int maxEval = numeric_limits<int>::min();
            for (const auto& child : node->getChildren()) {
                int eval = minimax(child.get(), depth + 1, alpha, beta, false);
                maxEval = max(maxEval, eval);
                alpha = max(alpha, eval);
                if (beta <= alpha) {
                    break;
                }
            }
            return maxEval;
        }
        else {
            int minEval = numeric_limits<int>::max();
            for (const auto& child : node->getChildren()) {
                int eval = minimax(child.get(), depth + 1, alpha, beta, true);
                minEval = min(minEval, eval);
                beta = min(beta, eval);
                if (beta <= alpha) {
                    break;
                }
            }
            return minEval;
        }
    }

    int findBestMove(GameTree* root, bool maximizingPlayer) {
        int bestValue = maximizingPlayer ? numeric_limits<int>::min() : numeric_limits<int>::max();
        int bestMove = -1;

        for (const auto& child : root->getChildren()) {
            nodeCount = 0;
            int childValue = minimax(child.get(), 0, -INF, INF, !maximizingPlayer);

            cout << "Ход " << child->move << ": оценка = " << childValue << ", узлов рассмотрено: " << nodeCount << endl;

            if (maximizingPlayer) {
                if (childValue > bestValue) {
                    bestValue = childValue;
                    bestMove = child->move;
                }
            }
            else {
                if (childValue < bestValue) {
                    bestValue = childValue;
                    bestMove = child->move;
                }
            }
        }

        cout << "Лучший ход: " << bestMove << " с оценкой " << bestValue << endl;
        return bestMove;
    }

    class ConnectFour {
    private:
        GameState currentState;
        int myColor;
        int opponentColor;

    public:
        ConnectFour(int myColor) : myColor(myColor), opponentColor(1 - myColor) {}

        void printGameInfo() {
            cout << "=== ИГРА 'ЧЕТЫРЕ В РЯД' ===" << endl;
            cout << "Глубина поиска: 4" << endl;
            cout << "Я играю за: " << (myColor == 0 ? "X" : "O") << endl;
            cout << "Текущее состояние доски:" << endl;
            currentState.printBoard();
            cout << endl;
        }

        void playGame() {
            printGameInfo();

            while (true) {
                // Проверяем состояние игры
                int gameState = currentState.checkWinner();
                if (gameState != -1) {
                    handleGameEnd(gameState);
                }

                // Ход противника (если нужно)
                if (currentState.currentPlayer == opponentColor) {
                    handleOpponentMove();

                    // Проверяем состояние после хода противника
                    gameState = currentState.checkWinner();
                    if (gameState != -1) {
                        handleGameEnd(gameState);
                    }
                }

                // Мой ход
                if (currentState.currentPlayer == myColor) {
                    handleMyMove();

                    // Проверяем состояние после моего хода
                    gameState = currentState.checkWinner();
                    if (gameState != -1) {
                        handleGameEnd(gameState);
                    }
                }
            }
        }

    private:
        void handleGameEnd(int gameState) {
            currentState.printBoard();
            if (gameState == myColor) {
                cout << "ПОБЕДА!" << endl;
                exit(0);
            }
            else if (gameState == opponentColor) {
                cout << "ПОРАЖЕНИЕ!" << endl;
                exit(3);
            }
            else if (gameState == 2) {
                cout << "НИЧЬЯ!" << endl;
                exit(4);
            }
        }

        void handleOpponentMove() {
            cout << "Ожидание хода противника..." << endl;
            string input;
            if (!getline(cin, input)) {
                cout << "Ошибка чтения хода противника" << endl;
                exit(1);
            }

            if (!input.empty()) {
                try {
                    int opponentMove = stoi(input);
                    if (currentState.isValidMove(opponentMove)) {
                        currentState.makeMove(opponentMove);
                        cout << "Противник сходил в колонку: " << opponentMove << endl;
                        currentState.printBoard();
                    }
                    else {
                        cout << "НЕВЕРНЫЙ ХОД ПРОТИВНИКА: " << opponentMove << endl;
                    }
                }
                catch (const exception& e) {
                    cout << "ОШИБКА: Неверный формат хода противника: " << input << endl;
                }
            }
        }

        void handleMyMove() {
            cout << "Мой ход..." << endl;

            // Строим дерево игры из текущего состояния
            auto root = make_unique<GameTree>(currentState);
            root->buildTree(0, 6); // Глубина поиска = 4 для быстрой игры, 6 - для лучшей

            int myMove = findBestMove(root.get(), myColor == 0 ? true : false);

            if (myMove != -1 && currentState.isValidMove(myMove)) {
                currentState.makeMove(myMove);
                cerr << myMove << endl; // Вывод хода в stderr
                cout << "Я сходил в колонку: " << myMove << endl;
                currentState.printBoard();
            }
            else {
                cout << "ОШИБКА: Невозможно сделать ход " << myMove << endl;
                // Пробуем любой валидный ход
                vector<int> moves = currentState.getValidMoves();
                if (!moves.empty()) {
                    myMove = moves[0];
                    currentState.makeMove(myMove);
                    cerr << myMove << endl;
                    cout << "Вынужденный ход в колонку: " << myMove << endl;
                    currentState.printBoard();
                }
            }
        }
    };

    int main(int argc, char* argv[]) {
        if (argc != 2) {
            cout << "Использование: " << argv[0] << " <цвет: 0 или 1>" << endl;
            cout << "0 - ходите первым (X)" << endl;
            cout << "1 - ходите вторым (O)" << endl;
            return 1;
        }

        int myColor;
        try {
            myColor = stoi(argv[1]);
            if (myColor != 0 && myColor != 1) {
                throw invalid_argument("Цвет должен быть 0 или 1");
            }
        }
        catch (const exception& e) {
            cout << "ОШИБКА: " << e.what() << endl;
            return 2;
        }

        ConnectFour game(myColor);
        game.playGame();

        return 0;
    }