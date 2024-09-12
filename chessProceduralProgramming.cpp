#include <iostream>
#include <vector>
using namespace std;

typedef struct {
    int row;
    int col;
} Slot;

string curPlayer = "White";
char board[8][8];
char boardCopy[8][8];
char lastReconstructableBoard[8][8];
char formerBoard[8][8];
int turns = 1;  // counts rounds, i.e., pairs of moves, not single moves
int numMovesWithoutCaptures = 0;
int numMovesFromLastPawnMove = 0;
bool whiteKingMoved, blackKingMoved = false;
bool whiteLeftRookMoved, whiteRightRookMoved = false;
bool blackLeftRookMoved, blackRightRookMoved = false;
// column of pawn that moved two slots in the last turn or -1 if no such pawn
int whiteEnPassant, blackEnPassant = -1;
vector<Slot> history;

void printBoard(bool printColumnsAndRow = 1) {
    for (int i = 7; i >= 0; i--) {
        if (printColumnsAndRow) {
            cout << i + 1 << " | ";
        }
        for (int j = 0; j < 8; j++) {
            cout << board[i][j] << ' ';
        }
        cout << endl;
    }
    if (printColumnsAndRow) {
        cout << "    ---------------" << endl;
        cout << "    A B C D E F G H" << endl;
    }
    cout << endl;
}

void initBoard() {
    board[0][0] = board[0][7] = 'R';
    board[0][1] = board[0][6] = 'K';
    board[0][2] = board[0][5] = 'B';
    board[1][0] = board[1][1] = board[1][2] = board[1][3] = 'P';
    board[1][4] = board[1][5] = board[1][6] = board[1][7] = 'P';
    board[0][3] = 'Q';
    board[0][4] = 'A';
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 2 == 0) {
                board[i][j] = '#';
            } else {
                board[i][j] = ' ';
            }
        }
    }
    board[6][0] = board[6][1] = board[6][2] = board[6][3] = 'p';
    board[6][4] = board[6][5] = board[6][6] = board[6][7] = 'p';
    board[7][0] = board[7][7] = 'r';
    board[7][1] = board[7][6] = 'k';
    board[7][2] = board[7][5] = 'b';
    board[7][3] = 'q';
    board[7][4] = 'a';
}

bool checkSlotInBoard(int row, int col) {
    if (row < 0 || row > 7 || col < 0 || col > 7) {
        return false;
    }
    return true;
}

void commitMove(char b[8][8], int cf, int rf, int ct, int rt) {
    bool isRealMove = b == board;
    if (isRealMove) {
        if (b[rt][ct] != ' ' && b[rt][ct] != '#') {  // hitting occurred
            numMovesWithoutCaptures = 0;
        } else {
            numMovesWithoutCaptures += 1;
        }

        if (b[rf][cf] == 'P' || b[rf][cf] == 'p') {  // a pawn moved
            numMovesFromLastPawnMove = 0;
        } else {
            numMovesFromLastPawnMove += 1;
        }

        // after king or rook moves, it can't castle anymore
        if (b[rf][cf] == 'A') {
            whiteKingMoved = true;
        } else if (b[rf][cf] == 'a') {
            blackKingMoved = true;
        // if any piece moves from the original location of a rook, either the
        // rook moves, or it has already moved, or it has already been hitted
        } else if (rf == 0 && cf == 0) {
            whiteLeftRookMoved = true;
        } else if (rf == 0 && cf == 7) {
            whiteRightRookMoved = true;
        } else if (rf == 7 && cf == 0) {
            blackLeftRookMoved = true;
        } else if (rf == 7 && cf == 7) {
            blackRightRookMoved = true;
        }

        // if pawn moves two slots, it might be hitted by en passant
        if (b[rf][cf] == 'P' && rf == 1 && rt == 3) {  // white pawn
            whiteEnPassant = cf;
        } else {  // en passant can occur only right after the pawn's move
            whiteEnPassant = -1;
        }
        if (b[rf][cf] == 'p' && rf == 6 && rt == 4) {  // black pawn
            blackEnPassant = cf;
        } else {
            blackEnPassant = -1;
        }
    }

    // if move is en passant, the hitted pawn should be removed; the pawn is
    // moved in the next code snippet
    if ((b[rf][cf] == 'P' || b[rf][cf] == 'p') && cf != ct &&
            (b[rt][ct] == ' ' || b[rt][ct] == '#')) {
        b[rf][ct] = (rf + cf) % 2 == 0? ' ' : '#';  // reconstruct hitted
    }                                               // pawn slot's color

    b[rt][ct] = b[rf][cf];  // move the piece
    b[rf][cf] = (rf + cf) % 2 == 0? '#' : ' ';

    // moving the rook if a castling occured
    if (b[rt][ct] == 'A' && cf == 4) {
        if (ct == 2) {  // king moved two slots, hence it must be castling
            commitMove(b, 0, 0, 3, 0);
            numMovesWithoutCaptures--;
            numMovesFromLastPawnMove--;
        } else if (ct == 6) {
            commitMove(b, 7, 0, 5, 0);
            numMovesWithoutCaptures--;
            numMovesFromLastPawnMove--;
        }
    }
    if (b[rt][ct] == 'a' && cf == 4) {
        if (ct == 2) {
            commitMove(b, 0, 7, 3, 7);
            numMovesWithoutCaptures--;
            numMovesFromLastPawnMove--;
        } else if (ct == 6) {
            commitMove(b, 7, 7, 5, 7);
            numMovesWithoutCaptures--;
            numMovesFromLastPawnMove--;
        }
    }

    if (isRealMove) {
        // promotion

        if (b[rt][ct] == 'P' && rt == 7) {  // white promotion
            char p;
            do {
                cout << "Choose pawn promotion - ";
                cout << "(K) knight, (B) bishop, (R) rook, (Q) queen: ";
                cin >> p;
            } while (p != 'K' && p != 'B' && p != 'R' && p != 'Q');
            b[rt][ct] = p;
        }
        if (b[rt][ct] == 'p' && rt == 0) {  // black promotion
            char p;
            do {
                cout << "Choose pawn promotion - ";
                cout << "(k) knight, (b) bishop, (r) rook, (q) queen: ";
                cin >> p;
            } while (p != 'k' && p != 'b' && p != 'r' && p != 'q');
            b[rt][ct] = p;
        }

        // update last reconstructable board if uncancelable move committed
        if (numMovesWithoutCaptures == 0 || numMovesFromLastPawnMove == 0) {
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    lastReconstructableBoard[i][j] = b[i][j];
                }
            }
        }
    }
}

vector<Slot> getPawnMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    int plusOrMinusOne = player == "White"? 1 : -1;
    int nextRow = rf + plusOrMinusOne;
    // check slot in next row and same column is empty
    if (b[nextRow][cf] == ' ' || b[nextRow][cf] == '#') {
        Slot s1;
        s1.row = nextRow, s1.col = cf;
        moves.push_back(s1);
        // if pawn hasn't moved and nothing in two next slots, add this move
        if ((player == "White" && rf == 1) ||
                (player == "Black" && rf == 6)) {
            int next2Row = nextRow + plusOrMinusOne;
            if (b[next2Row][cf] == ' ' || b[next2Row][cf] == '#') {
                Slot s2;
                s2.row = next2Row, s2.col = cf;
                moves.push_back(s2);
            }
        }
    }
    // check for hitting options
    char otherMinLetter = player == "White"? 'a' : 'A';
    char otherMaxLetter = player == "White"? 'z' : 'Z';
    if (checkSlotInBoard(rf + plusOrMinusOne, cf - 1)) {
        if (b[rf + plusOrMinusOne][cf - 1] >= otherMinLetter &&
                b[rf + plusOrMinusOne][cf - 1] <= otherMaxLetter) {
            Slot s3;
            s3.row = rf + plusOrMinusOne, s3.col = cf - 1;
            moves.push_back(s3);
        }
    }
    if (checkSlotInBoard(rf + plusOrMinusOne, cf + 1)) {
        if (b[rf + plusOrMinusOne][cf + 1] >= otherMinLetter &&
                b[rf + plusOrMinusOne][cf + 1] <= otherMaxLetter) {
            Slot s4;
            s4.row = rf + plusOrMinusOne, s4.col = cf + 1;
            moves.push_back(s4);
        }
    }
    // en passant (white and than black)
    if (blackEnPassant != -1 && rf == 4 &&
            (cf - blackEnPassant == 1 || cf - blackEnPassant == -1)) {
        Slot s5;
        s5.row = 5, s5.col = blackEnPassant;
        moves.push_back(s5);
    }
    if (whiteEnPassant != -1 && rf == 3 &&
            (cf - whiteEnPassant == 1 || cf - whiteEnPassant == -1)) {
        Slot s6;
        s6.row = 2, s6.col = whiteEnPassant;
        moves.push_back(s6);
    }
    return moves;
}

vector<Slot> getKnightMoves(char b[8][8], int rf, int cf, string player) {
    char minLetter = player == "White"? 'A' : 'a';
    char maxLetter = player == "White"? 'Z' : 'z';
    vector<Slot> moves;
    Slot eightOptions[8] = {
        Slot(), Slot(), Slot(), Slot(), Slot(), Slot(), Slot(), Slot()
    };
    eightOptions[0].col = cf - 2, eightOptions[0].row = rf - 1;
    eightOptions[1].col = cf - 2, eightOptions[1].row = rf + 1;
    eightOptions[2].col = cf - 1, eightOptions[2].row = rf - 2;
    eightOptions[3].col = cf - 1, eightOptions[3].row = rf + 2;
    eightOptions[4].col = cf + 1, eightOptions[4].row = rf - 2;
    eightOptions[5].col = cf + 1, eightOptions[5].row = rf + 2;
    eightOptions[6].col = cf + 2, eightOptions[6].row = rf - 1;
    eightOptions[7].col = cf + 2, eightOptions[7].row = rf + 1;
    for (int i = 0; i < 8; i++) {
        char r = eightOptions[i].row, c = eightOptions[i].col;
        if (!checkSlotInBoard(r, c) ||
                // can't hit piece of same color
                (b[r][c] >= minLetter && b[r][c] <= maxLetter)) {
            continue;
        }
        moves.push_back(eightOptions[i]);
    }
    return moves;
}

vector<Slot> getOneDirMoves(char b[8][8], int rf, int cf, char otherMinLetter,
                          char otherMaxLetter, int rowDir, int colDir) {
    vector<Slot> moves;
    char inSlot;
    int distance = 1;
    while (checkSlotInBoard(rf + distance * rowDir, cf + distance * colDir)) {
        inSlot = b[rf + distance * rowDir][cf + distance * colDir];
        if (inSlot == ' ' || inSlot == '#') {
            Slot s;
            s.row = rf + distance * rowDir;
            s.col = cf + distance * colDir;
            moves.push_back(s);
        } else {
            if (inSlot >= otherMinLetter && inSlot <= otherMaxLetter) {
                Slot s;
                s.row = rf + distance * rowDir;
                s.col = cf + distance * colDir;
                moves.push_back(s);
            }
            break;
        }
        distance++;
    }
    return moves;
}

vector<Slot> getBishopMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    char otherMinLetter = player == "White"? 'a' : 'A';
    char otherMaxLetter = player == "White"? 'z' : 'Z';
    vector<Slot> diags[4];
    diags[0] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, 1);
    diags[1] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, 1);
    diags[2] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, -1);
    diags[3] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, -1);
    for (vector<Slot> diag : diags) {
        for (Slot s : diag) {
            moves.push_back(s);
        }
    }
    return moves;
}

vector<Slot> getRookMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    char otherMinLetter = player == "White"? 'a' : 'A';
    char otherMaxLetter = player == "White"? 'z' : 'Z';
    vector<Slot> diags[4];
    diags[0] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, 0);
    diags[1] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 0, 1);
    diags[2] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, 0);
    diags[3] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 0, -1);
    for (vector<Slot> diag : diags) {
        for (Slot s : diag) {
            moves.push_back(s);
        }
    }
    return moves;
}

vector<Slot> getQueenMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    char otherMinLetter = player == "White"? 'a' : 'A';
    char otherMaxLetter = player == "White"? 'z' : 'Z';
    vector<Slot> diags[8];
    diags[0] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, 0);
    diags[1] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 0, 1);
    diags[2] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, 0);
    diags[3] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 0, -1);
    diags[4] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, 1);
    diags[5] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, 1);
    diags[6] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, 1, -1);
    diags[7] = getOneDirMoves(b, rf, cf, otherMinLetter, otherMaxLetter, -1, -1);
    for (vector<Slot> diag : diags) {
        for (Slot s : diag) {
            moves.push_back(s);
        }
    }
    return moves;
}

bool checkOneKingMove(char b[8][8], int r, int c, char otherMinLetter,
                      char otherMaxLetter) {
    if (checkSlotInBoard(r, c)) {
        if (b[r][c] == ' ' || b[r][c] == '#') {
            return true;
        } else if (b[r][c] >= otherMinLetter &&
                   b[r][c] <= otherMaxLetter) {
            return true;
        }
    }
    return false;
}

vector<Slot> getKingMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    char otherMinLetter = player == "White"? 'a' : 'A';
    char otherMaxLetter = player == "White"? 'z' : 'Z';
    if(checkOneKingMove(b, rf + 1, cf - 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf + 1, s.col = cf - 1;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf + 1, cf, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf + 1, s.col = cf;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf + 1, cf + 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf + 1, s.col = cf + 1;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf, cf - 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf, s.col = cf - 1;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf, cf + 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf, s.col = cf + 1;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf - 1, cf - 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf - 1, s.col = cf - 1;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf - 1, cf, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf - 1, s.col = cf;
        moves.push_back(s);
    }
    if(checkOneKingMove(b, rf - 1, cf + 1, otherMinLetter, otherMaxLetter)) {
        Slot s;
        s.row = rf - 1, s.col = cf + 1;
        moves.push_back(s);
    }
    // castling
    if (rf == 0 && !whiteKingMoved) {  // white castling
        // castling with left rook
        if (!whiteLeftRookMoved && (b[0][1] == ' ' || b[0][1] == '#') &&
            (b[0][2] == ' ' || b[0][2] == '#') &&
            (b[0][3] == ' ' || b[0][3] == '#')) {
                Slot s;
                s.row = 0, s.col = 2;
                moves.push_back(s);
        }
        // castling with right rook
        if (!whiteRightRookMoved && (b[0][6] == ' ' || b[0][6] == '#') &&
            (b[0][5] == ' ' || b[0][5] == '#')) {
                Slot s;
                s.row = 0, s.col = 6;
                moves.push_back(s);
        }
    }
    if (rf == 7 && !blackKingMoved) {  // black castling
        // castling with left rook
        if (!blackLeftRookMoved && (b[7][1] == ' ' || b[7][1] == '#') &&
            (b[7][2] == ' ' || b[7][2] == '#') &&
            (b[7][3] == ' ' || b[7][3] == '#')) {
                Slot s;
                s.row = 7, s.col = 2;
                moves.push_back(s);
        }
        // castling with right rook
        if (!blackRightRookMoved && (b[7][6] == ' ' || b[7][6] == '#') &&
            (b[7][5] == ' ' || b[7][5] == '#')) {
                Slot s;
                s.row = 7, s.col = 6;
                moves.push_back(s);
        }
    }
    return moves;
}

vector<Slot> getOptionalMoves(char b[8][8], int rf, int cf, string player) {
    vector<Slot> moves;
    switch (b[rf][cf]) {
        case 'P':
        case 'p':
            moves = getPawnMoves(b, rf, cf, player);
            break;
        case 'K':
        case 'k':
            moves = getKnightMoves(b, rf, cf, player);
            break;
        case 'B':
        case 'b':
            moves = getBishopMoves(b, rf, cf, player);
            break;
        case 'R':
        case 'r':
            moves = getRookMoves(b, rf, cf, player);
            break;
        case 'Q':
        case 'q':
            moves = getQueenMoves(b, rf, cf, player);
            break;
        case 'A':
        case 'a':
            moves = getKingMoves(b, rf, cf, player);
            break;
    }
    return moves;
}

bool isCheck(char b[8][8], string player = curPlayer) {
    char minLetter = player == "White"? 'A' : 'a';
    char maxLetter = player == "White"? 'Z' : 'z';
    char otherKing = player == "White"? 'a' : 'A';
    // look for player's pieces
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // check if the other king is in the optional moves of some piece
            if (b[i][j] >= minLetter && b[i][j] <= maxLetter) {
                for (Slot s : getOptionalMoves(b, i, j, player)) {
                    if (b[s.row][s.col] == otherKing) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool isCheckAfterMove(int rf, int cf, int rt, int ct) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            boardCopy[i][j] = board[i][j];
        }
    }

    commitMove(boardCopy, cf, rf, ct, rt);

    string player = curPlayer == "White"? "Black" : "White";
    return isCheck(boardCopy, player);
}

bool isMoveLegal(int cf, int rf, int ct, int rt) {
    // check if source and destination are in board
    if (!checkSlotInBoard(cf, rf) ||
            !checkSlotInBoard(ct, rt)) {
        return false;
    }
    // check the source slot contains a piece
    if (board[rf][cf] == ' ' || board[rf][cf] == '#') {
        return false;
    }
    // check piece in the slot belongs to current player
    if (curPlayer == "White") {
        switch (board[rf][cf]) {
            case 'p':
            case 'k':
            case 'b':
            case 'r':
            case 'q':
            case 'a':
                return false;
        }
    } else {
        switch (board[rf][cf]) {
            case 'P':
            case 'K':
            case 'B':
            case 'R':
            case 'Q':
            case 'A':
                return false;
        }
    }
    // check if destination is in the optional moves of the piece
    vector<Slot> optionalMoves = getOptionalMoves(board, rf, cf, curPlayer);
    bool moveIsOptional = false;
    for (Slot s : optionalMoves) {
        if (s.row == rt && s.col == ct) {
            moveIsOptional = true;
            break;
        }
    }
    if (!moveIsOptional) {
        return false;
    }
    // check if piece can't move for check occurs/left after the move
    if(isCheckAfterMove(rf, cf, rt, ct)) {
        return false;
    }
    
    return true;
}

bool checkTouchMove(int cf, int rf) {
    // check slot exists
    if (!checkSlotInBoard(cf, rf)) {
        return false;
    }
    // check slot contains a piece
    char inSlot = board[rf][cf];
    if (inSlot == ' ' || inSlot == '#') {
        return false;
    }
    // check the piece belongs to current player
    if ((curPlayer == "White" && !(inSlot >= 'A' && inSlot <= 'Z')) || 
        (curPlayer == "Black" && !(inSlot >= 'a' && inSlot <= 'z'))) {
        return false;
    }
    // check the piece has legal moves to make
    for (Slot s : getOptionalMoves(board, rf, cf, curPlayer)) {
        if (isMoveLegal(cf, rf, s.col, s.row)) {
            return true;
        }
    }
    return false;
}

bool isCheckmate() {
    // TODO
    // inefficient implementation: check if the check still holds for all legal moves of the threatened player
    // remark: checkmate can be done only by pawn in diagonal to the king, knight in distance of 2 rows/columns and
    //         1 column/row from the king, bishop, rook, and queen.
    return false;
}

bool askForDrawClaiming() {
    char answer;
    string playersColors[] = {"White", "Black"};
    for (string player : playersColors) {
        do {
            cout << player << ", would you like to claim a draw? (y) yes, (n) no";
            cin >> answer;
        } while (answer != 'y' && answer != 'n');
        if (answer == 'y') {
            return true;
        }
    }
    return false;
}

bool isDraw() {
    // TODO:
    // 1. stalemate (no legal move for the current player)
    // 2. threefold repitition: if current situation has already occurred twice, not necessarily in the last
    //    positions, each player can reclaim a draw (use askForDrawClaiming for questioning), and in fivefold
    //    repitition a draw is enforced
    if (numMovesWithoutCaptures == 50 || numMovesFromLastPawnMove == 50) {
        if (askForDrawClaiming()) {
            return true;
        }
    } else if (numMovesWithoutCaptures == 75 || numMovesFromLastPawnMove == 75) {
        return true;
    }
    return false;
}

void startGame() {
    char colFrom, colTo;
    int rowFrom, rowTo, cf, ct;
    bool moveLegality;

    initBoard();
    printBoard();

    while (true) {  // break occurs in case of checkmate or draw
        rowFrom = -1;  // preventing touch-move for the first move input
        do {  // get source and destination from user until they're legal
            if (rowFrom == -1) {  // no touch-move occured
                cout << '(' << turns << ") " << curPlayer << ": ";
                cin >> colFrom >> rowFrom >> colTo >> rowTo;
                // turn slots into indices
                cf = (int) colFrom - 'a';  // [a,h] => [0,7]
                rowFrom--;  // [1,8] => [0,7]
            } else {  // touch-move - source slot is already known
                cout << '(' << turns << ") " << curPlayer << ": ";
                cout << colFrom << rowFrom + 1 << ' ';
                cin >> colTo >> rowTo;  // should insert just destination slot
            }
            // turn slots into indices
            ct = (int) colTo - 'a';  // [a,h] => [0,7]
            rowTo--;  // [1,8] => [0,7]
            moveLegality = isMoveLegal(cf, rowFrom, ct, rowTo);
            if (!checkTouchMove(cf, rowFrom)) {
                rowFrom = -1;
            }
        } while (!moveLegality);
        cout << endl;

        commitMove(board, cf, rowFrom, ct, rowTo);

        printBoard();

        // document step in history for threefold/fivefold repeatition check
        Slot move;
        move.col = colTo, move.row = rowTo;
        history.push_back(move);

        if (isCheck(board)) {  // checking for check, checkmate, and draw
            if (isCheckmate()) {
                cout << "Checkmate, " << curPlayer << " won!" << endl;
                break;
            } else {
                cout << "Check." << endl;
            }
        } else if (isDraw()) {
            cout << "Draw!" << endl;
            break;
        }

        if (curPlayer == "White") {  // change player's turn
            curPlayer = "Black";
        } else {
            curPlayer = "White";
            turns += 1;
        }

    }
}

int main() {
    startGame();

    return 0;
}
