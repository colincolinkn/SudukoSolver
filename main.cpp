//
//  main.cpp
//  SudokuSolver
//
//  Created by Kangning Chen on 2015-07-03.
//  Copyright (c) 2015 Kangning Chen. All rights reserved.
//

#include <iostream>
#include <vector>

using namespace std;
enum CellStatus {
    FIXED,
    EMPTY,
    FILLED,
    TRIED
};

struct Cell {
    
    int locks[10]; // indicate how many locks by neighbors that this cell cannot be set,
    int val;
    
    CellStatus status;
    int x;
    int y;
    bool isAvailbleAt(int val) {
        return (status == EMPTY && locks[val] == 0);
    }
    
    int remainingProb;
    bool setVal(int _val, CellStatus _status) {
        if (isAvailbleAt(_val)) {
            val = _val;
            status = _status;
            return true;
        } else {
            return false;
        }
    }
    
    void findVal(int& _x, int& _y, int& _val) {
        if (this->remainingProb >= 1 && this->status == CellStatus::EMPTY) {
            for (int i = 1; i <= 9; i++) {
                if (locks[i] == 0) {
                    //cout << "ONLY VAL: " << x << " " << y << " " << i << endl;
                    _x = this->x;
                    _y = this->y;
                    _val = i;
                    return;
                }
            }
        }
    }
    
    void addPosiCount(vector<int>& posi) {
        if (this->status != CellStatus::EMPTY)  {
            posi[this->val] = 10;
            return;
        }
        for (int i = 1; i <= 9; i++) {
            if (locks[i] == 0) {
                posi[i]++;
            }
        }
    }
    
    int getVal() {
        return val;
    }
    
    void addLock(int _val) {
        if (status == CellStatus::EMPTY) {
            if (locks[_val] == 0) {
                remainingProb--;
            }
            locks[_val]++;
        }
    }
    
    void removeLock(int _val) {
        if (status == CellStatus::EMPTY) {
            locks[_val]--;
            if (locks[_val] == 0) {
                remainingProb++;
            }
        }
    }
    
    void print() {
        if (val > 0 && this->status != CellStatus::EMPTY) {
            cout << val;
        } else {
            cout << ' ';
        }
    }
    
    Cell(int _x, int _y) :x(_x), y(_y), remainingProb(9),status(EMPTY), val(0){
        for (int i = 0; i < 10; i++) {
            locks[i] = 0;
        }
    }
};

enum ComboType {
    ROW,
    COLUMN,
    SQUARE,
};

struct Combo {
    Cell* cellList[9];
    int idx;
    ComboType type;
    
    bool checkValidate() {
        bool checkList[10];
        for (int j = 1; j <= 9; j++) {
            checkList[j] = false;
        }
        for (int j = 0; j < 9; j++) {
            int val = cellList[j]->getVal();
            if (val > 0 && checkList[val]) {
                cout << "ERROR::  Type: " << type << " IDX: " << idx  << " VAL: " << val<< endl;
                return false;
            } else {
                checkList[val] = true;
            }
        }
        return true;
    }
    
    int determines(int& x, int& y, int& val) {
        vector<int> checkList(10, 0);
        for (int i = 0; i < 9; i++) {
            cellList[i]->addPosiCount(checkList);
        }
        
        int min_choice = 9;
        int min_idx = -1;
        for (int i = 1; i <= 9; i++) {
            if (checkList[i] < min_choice) {
                min_choice = checkList[i];
                min_idx = i;
            }
        }
        
        if (min_choice == 0) {
            return 0;
        } else if (min_choice == 1) {
            for (int j = 0; j < 9; j++) {
                if (cellList[j]->isAvailbleAt(min_idx)) {
                    //cout << "Possible:: Type: " << type << " IDX: " << idx  << " VAL: " << min_idx << endl;
                    x = cellList[j]->x;
                    y = cellList[j]->y;
                    val = min_idx;
                    return 1;
                }
            }
        }
        
        return 2;
    }
};

class Board {
    static Board *instance;
    Cell* grid[9][9];
    
    vector<Combo> combos;
    vector<Cell*> dfs_record;
    
    //it->first: parent, it->second.first: child, it->second.second: child value
    vector<pair<Cell*, pair<Cell*, int>>> dfs_prev_locks;
    
    Board() {
        dfs_record.clear();
        dfs_prev_locks.clear();
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                Cell* cell = new Cell(i, j);
                grid[i][j] = cell;
            }
        }
        
        for (int i = 0; i < 9; i++) {
            Combo cb;
            cb.type = ComboType::ROW;
            cb.idx = i;
            for (int j = 0; j < 9; j++) {
                cb.cellList[j] = grid[i][j];
            }
            this->combos.push_back(cb);
        }
        
        for (int i = 0; i < 9; i++) {
            Combo cb;
            cb.type = ComboType::COLUMN;
            cb.idx = i;
            for (int j = 0; j < 9; j++) {
                cb.cellList[j] = grid[j][i];
            }
            this->combos.push_back(cb);
        }
        
        for (int i = 0; i < 9; i++) {
            Combo cb;
            cb.type = ComboType::SQUARE;
            cb.idx = i;
            for (int j = 0; j < 9; j++) {
                cb.cellList[j] = grid[i/3*3+j/3][i%3*3+j%3];
            }
            this->combos.push_back(cb);
        }
    }
    
    ~Board() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                delete grid[i][j];
            }
        }
    }
public:
    
    bool isFull() {
        return  (dfs_record.size() == 81);
    }
    
    bool dfsBacktrace() {
        //this->printGrid();
        if (dfs_record.empty()) {
            // Ok.. no solution at all
            cout << "Tried.. No solution..." << endl;
            return false;
        }
        Cell *c = dfs_record.back();
        
        if (c->status == FILLED) {
            // Ok.. no solution at all
            cout << "Tried.. No solution..." << endl;
            return false;
        }
        
        int _x = c->x;
        int _y = c->y;
        int _val = c->val;
        c->status = CellStatus::EMPTY;
        c->val = 0;
        //cout << "Delete Val at: " << _x << " " << _y << ": " << _val << endl;
        c->addLock(_val);
        
        dfs_record.pop_back();
        
        while (!this->dfs_prev_locks.empty()) {
            pair<Cell*, pair<Cell*, int>> p = dfs_prev_locks.back();
            if (p.first != dfs_record.back()) {
                p.second.first->removeLock(p.second.second);
                dfs_prev_locks.pop_back();
            } else {
                break;
            }
        }
        this->dfs_prev_locks.push_back(pair<Cell*, pair<Cell*, int>>(dfs_record.back(), pair<Cell*, int>(c, _val)));
        
        notifyNeighbors(_x, _y, _val, false);
        //this->printGrid();
        return true;
    }
    
    void notifyNeighbors(int x, int y, int val, bool addLocks) {
        for (int k = 0; k < 9; k++) {
            if (k/3 != y/3) {
                if (addLocks) {
                    grid[x][k]->addLock(val);
                } else {
                    grid[x][k]->removeLock(val);
                }
            } if (k/3 != x/3) {
                if (addLocks) {
                    grid[k][y]->addLock(val);
                } else {
                    grid[k][y]->removeLock(val);
                }
            } if (k/3 != x%3 || k%3 != y%3) {
                if (addLocks) {
                    grid[x/3*3+k/3][y/3*3+k%3]->addLock(val);
                } else {
                    grid[x/3*3+k/3][y/3*3+k%3]->removeLock(val);
                }
            }
        }
    }
    
    bool setVal(int i, int j, int val, CellStatus status) {
        bool succeed = grid[i][j]->setVal(val, status);
        if (!succeed) {
            //cout << "Fail to set val: " << i << " " << j << " " << val << endl;
            return false;
        } else {
            //cout << "Set Val at: " << i << " " << j << ": " << val << endl;
        }
        dfs_record.push_back(grid[i][j]);
        notifyNeighbors(i, j, val, true);
        return true;
    }
    
    bool dfsNext() {
        int min_choice_by_combo = 9;
        int min_x = 0, min_y = 0, min_val = 0;
        /*
         * Try to find determined values by combos
         */
        int idx = -1;
        for (int i = 0; i < this->combos.size(); i++) {
            int x, y, val, scd;
            scd = this->combos[i].determines(x, y, val);
            if (scd < min_choice_by_combo) {
                min_choice_by_combo = scd;
                min_x = x;
                min_y = y;
                min_val = val;
                idx = i;
            }
        }
        
        if (min_choice_by_combo == 0) {
            return this->dfsBacktrace();
        }
        
        /*
         * Try to find determined values by cells
         */
        Cell *c = NULL;
        int min_choice_by_cell = 9;
        int j = -1;
        for (int i = 0; i < 81; i++) {
            if (grid[i/9][i%9]->status == CellStatus::EMPTY && grid[i/9][i%9]->remainingProb < min_choice_by_cell) {
                c = grid[i/9][i%9];
                min_choice_by_cell = grid[i/9][i%9]->remainingProb;
                j = i;
            }
        }
        
        if (min_choice_by_cell == 0) {
            //cout << "No posibility at " << j/9 << j%9 << endl;
            return this->dfsBacktrace();
        }
        
        if (min_choice_by_combo == 1) {
            this->setVal(min_x, min_y, min_val, CellStatus::FIXED);
            //printGrid();
            return true;
        }
        
        if (min_choice_by_cell == 1) {
            int x, y, val;
            c->findVal(x, y, val);
            this->setVal(x, y, val, CellStatus::FIXED);
            //printGrid();
            return true;
        } else {
            int x, y, val;
            c->findVal(x, y, val);
            this->setVal(x, y, val, CellStatus::TRIED);
            //printGrid();
            return true;
        }
    }
    
    bool checkValidate() {
        for (int i = 0; i < this->combos.size(); i++) {
            if (!this->combos[i].checkValidate()) {
                return false;
            }
        }
        return true;
    }
    void printGrid() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid[i][j]->print();
            }
            cout << endl;
        }
    }
    
    void readInput() {
        string x;
        for (int i = 0; i < 9; i++) {
            cin >> x;
            for (int j = 0; j < 9; j++) {
                char c = x[j];
                if (c >= '1' && c <= '9') {
                    this->setVal(i, j, c - '0', CellStatus::FILLED);
                }
            }
        }
    }
    
    static Board* getInstance() {
        if (instance == NULL) {
            instance = new Board();
        }
        return instance;
    }
    
    void release() {
        if (instance != NULL) {
            delete instance;
            instance = NULL;
        }
    }
};

Board* Board::instance = NULL;

int main(int argc, const char * argv[]) {
    Board* b = Board::getInstance();
    string cmd;
    while (cin >> cmd) {
        if (cmd == "game"||cmd =="g") {
            if (b != NULL){
                b->release();
                b=Board::getInstance();
            }
            b->readInput();
            b->printGrid();
        } else if (cmd == "quit"||cmd =="q") {
            break;
        } else if (cmd == "continue"||cmd=="c") {
            clock_t start, finish;
            double duration;
            start = clock();
            if (!b->checkValidate()) {
                cout << "Start Sudoku is Wrong" << endl;
            }
            while (!b->isFull()) {
                bool failed = !b->dfsNext();
                if (failed) {
                    cout << "No Solution" << endl;
                    break;
                }
            }
            b->printGrid();
            finish = clock();
            duration = (double)(finish - start) / CLOCKS_PER_SEC;
            printf( "Solved problem in %f seconds\n", duration );
        } else if (cmd == "next" || cmd =="n") {
            b->dfsNext();
            b->printGrid();
        }
    }
    b->release();
    return 0;
}
