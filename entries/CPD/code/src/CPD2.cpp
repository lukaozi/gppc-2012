/* 
 * File:   CPD2.cpp
 * Author: abotea
 * 
 * Created on 29 August 2011, 11:49 AM
 */

#include "CPD2.h"
#include "globals.h"
#include <iostream>
#include <fstream>
#include "utils2.h"
#include <assert.h>

using namespace std;

CPD2::CPD2() {
    for (int i = 0; i < 5; i++)
        this->repetitions_left[i] = 0;
}

CPD2::CPD2(const CPD2& orig) {
}

CPD2::~CPD2() {
    for (int i = 0; i < 5; i++) {
        delete []data[i];
        delete []heads[i];
    }
    delete []default_moves;
    delete []list_length;
}

void CPD2::readDataFromStream2(ifstream & infile) {
    infile.read((char*) &nr_nodes, sizeof (int));
    default_moves = new char[2 * nr_nodes];
    for (int k = 0; k < 2 * nr_nodes; k += 2) {
        infile.read(default_moves + k, 2);
    }
    list_length = new int[4*nr_nodes];
    infile.read((char*) list_length, 4 * nr_nodes * sizeof (int));
    // finished reading data written by the DeMo program
    // now start reading data written by SuCo repair
    for (int k = 0; k < 5; k++) {
        // read the head indexes
        infile.read((char*) &(nr_heads[k]), sizeof (int));
        //cerr << "offset: " << infile.tellg() << endl;
        //cerr << k << " " << nr_heads[k] << " " << nr_nodes << endl;
        assert (nr_heads[k] == 4*nr_nodes);
        heads[k] = new int [nr_heads[k]];
        infile.read((char*) (heads[k]), nr_heads[k] * sizeof (int));
        //cerr << "offset: " << infile.tellg() << endl;
        // read the data block
        infile.read((char*) &(block_size[k]), sizeof (int));
        //cerr << "offset: " << infile.tellg() << endl;
        data[k] = new char [block_size[k]];
        infile.read(data[k], block_size[k]);
        //cerr << "offset: " << infile.tellg() << endl;
    }
    assert(nr_heads[0] == nr_heads[1] && nr_heads[1] == nr_heads[2]);
    infile.close();
}

int CPD2::getMove(int startIdx, int targetIdx, int & nr_rect_checks,
        const AbstractGraph * graph) {
    nr_rect_checks = 0;
    int target_col = (graph->getNode(targetIdx))->getColumn();
    int target_row = graph->getNode(targetIdx)->getRow();
    int origin_col = (graph->getNode(startIdx))->getColumn();
    int origin_row = graph->getNode(startIdx)->getRow();
    int quarter = getQuarter(target_col, target_row, origin_col, origin_row);
    for (int i = 0; i < 5; i++) {
        this->initComponentStack(startIdx, i, quarter);
    }
    while (true) {
        // if we've parsed the whole list of rectangles, return default move
        if (type0_atoms_read[0] >= this->list_length[4*startIdx+quarter])
            return getDefaultMove(startIdx, targetIdx, graph);
        nr_rect_checks++;
        // left column
        unsigned int lc = getNextValueAndUpdateStack(0);
        // upper row
        unsigned int ur = getNextValueAndUpdateStack(1);
        // right column
        unsigned int rc = getNextValueAndUpdateStack(2);
        // bottom row
        unsigned int br = getNextValueAndUpdateStack(3);
        // move
        unsigned int mv = getNextValueAndUpdateStack(4);
        //cerr << "rectangle " << lc << " " << ur << " " << rc << " " << br << " " << mv << endl;
        if (ur == 0 && rc == 0 && br == 0) {
            int idx = lc;
            lc = graph->getNode(idx)->getColumn();
            ur = graph->getNode(idx)->getRow();
            if (target_col == lc && target_row == ur)
                return mv;
            else
            	continue;
        }
        if (target_col < lc)
            continue;
        if (target_row < ur)
            continue;
        if (rc < target_col)
            continue;
        if (br < target_row)
            continue;
        return mv;
        /*
        if (lc <= target_col && rc >= target_col &&
                ur <= target_row && br >= target_row)
            return mv;
         */
    }
}

int CPD2::getDefaultMove(int stIdx, int tgIdx, const AbstractGraph *gr) const {
    int target_col = gr->getNode(tgIdx)->getColumn();
    int target_row = gr->getNode(tgIdx)->getRow();
    int origin_col = gr->getNode(stIdx)->getColumn();
    int origin_row = gr->getNode(stIdx)->getRow();
    if (target_col <= origin_col) {
        if (target_row <= origin_row)
            return (default_moves[2 * stIdx] >> 4) & 15; // quarter 0
        else
            return default_moves[2 * stIdx + 1] & 15; // quarter 3
    } else {
        if (target_row <= origin_row)
            return default_moves[2 * stIdx] & 15; // quarter 1
        else
            return (default_moves[2 * stIdx + 1] >> 4) & 15; // quarter 2
    }
}

unsigned int CPD2::getNextValueAndUpdateStack2(int component) {
    // this the case N [X] i.e., a number N repeating X times
    if (repetitions_left[component] > 0) {
        repetitions_left[component]--;
        type0_atoms_read[component]++;
        return most_recent_value[component];
    }
    while (!st[component].empty()) {
        StackElement se = st[component].top();
        // if read in the most recent jump as many as we wanted:
        if (se.max_atoms_to_read <= se.atoms_read) {
            unsigned int read_recently = se.atoms_read;
            st[component].pop();
            if (!st[component].empty())
                st[component].top().atoms_read += read_recently;
        } else // we reached a case where we need to read more atoms
            break;
    }
    // if stack is empty, we are done; therefore, return NO_VALUE
    // to indicate to the calling method that the default move should be used
    if (st[component].empty()) {
        assert(false); // we handle the default move case in getMove()
        return NO_VALUE;
    }
    StackElement se = st[component].top();
    unsigned int type = getTypeOfRecord(component);
    if (type == 0) {
        // the type where we read the value directly here (3 bytes)
        unsigned int result = (data[component][se.byte_idx + 2] & 255);
        result |= ((data[component][se.byte_idx + 1] & 255) << 8);
        // why & 31: ignore first 2 bits, as they encode the type:
        result |= ((data[component][se.byte_idx] & 31) << 16);
        st[component].top().byte_idx += 3; // we've consumed three bytes
        st[component].top().atoms_read++;
        type0_atoms_read[component]++;
        most_recent_value[component] = result;
        return result;
    } else if (type == 1) {
        // the type [X] i.e., we read how many times the previously read value will repeat
        unsigned int value = data[component][se.byte_idx] & 63; //&31;
        // one repetition used already when we read the value itself:
        st[component].top().atoms_read++;
        repetitions_left[component] = value - 1;
        assert(repetitions_left[component] >= 0);
        st[component].top().byte_idx += 1; // we read one more byte
        // next call will return right after executing the part at the top
        //return getNextValueAndUpdateStack(component);
        repetitions_left[component]--;
        type0_atoms_read[component]++;
        return most_recent_value[component];
    } else if (type == 2) { // case ( offset substring_length ) :
        // read offset in value1:
        unsigned int value1 = data[component][se.byte_idx + 3] & 255;
        value1 |= ((data[component][se.byte_idx + 2] & 255) << 8);
        value1 |= ((data[component][se.byte_idx + 1] & 255) << 16);
        value1 |= (data[component][se.byte_idx] & 1) << 24;
        //value1 |= ((data[component][se.iterator] & 32) << 19);
        assert(value1 < se.byte_idx);
        value1 = se.byte_idx - value1;
        // read length of substring (i.e., # atoms) in value2:
        unsigned int value2 = ((data[component][se.byte_idx] & 255) >> 1) & 31; //15;
        st[component].top().byte_idx += 4; // we've consumed 4 more bytes
        unsigned int mar = min(value2, se.max_atoms_to_read - se.atoms_read);
        st[component].push(StackElement(value1, mar));
        return getNextValueAndUpdateStack(component);
    } else if (type == 3) { // case (offset substring_length ) with substring_length = 2 or 3
        unsigned int value1 = data[component][se.byte_idx + 2] & 255;
        value1 |= ((data[component][se.byte_idx + 1] & 255) << 8);
        value1 |= ((data[component][se.byte_idx] & 31) << 16);
        assert(value1 <= se.byte_idx);
        value1 = se.byte_idx - value1;
        unsigned int value2 = 2 + ((data[component][se.byte_idx] & 32) >> 5);
        st[component].top().byte_idx += 3; // we've consumed 3 more bytes
        unsigned int mar = min(value2, se.max_atoms_to_read - se.atoms_read);
        st[component].push(StackElement(value1, mar));
        return getNextValueAndUpdateStack(component);
    } else {
        assert(false);
    }
}

unsigned int CPD2::getNextValueAndUpdateStack(int component) {
    while (true) {
        // this the case N [X] i.e., a number N repeating X times
        if (repetitions_left[component] > 0) {
            repetitions_left[component]--;
            type0_atoms_read[component]++;
            return most_recent_value[component];
        }
        StackElement se = st[component].top();
        // if read in the most recent jump as many as we wanted:
        if (se.max_atoms_to_read <= se.atoms_read) {
            unsigned int read_recently = se.atoms_read;
            st[component].pop();
            if (!st[component].empty())
                st[component].top().atoms_read += read_recently;
            continue;
        }
        unsigned int type = getTypeOfRecord(component);
        if (type == 0) {
            if (component == 4) { // move records
                // the type where we read the value directly here (3 bytes)
                unsigned int result = (data[component][se.byte_idx] & 63);
                // why & 31: ignore first 2 bits, as they encode the type:
                // SHOULDN'T IT BE 63 THEN???
                st[component].top().byte_idx += 1; // we've consumed one byte
                st[component].top().atoms_read++;
                type0_atoms_read[component]++;
                most_recent_value[component] = result;
                return result;
            } else { // 3-byte records
                // the type where we read the value directly here (3 bytes)
                unsigned int result = (data[component][se.byte_idx + 2] & 255);
                result |= ((data[component][se.byte_idx + 1] & 255) << 8);
                // why & 31: ignore first 2 bits, as they encode the type:
                // SHOULDN'T IT BE 63 THEN???
                result |= ((data[component][se.byte_idx] & 63) << 16);
                st[component].top().byte_idx += 3; // we've consumed three bytes
                st[component].top().atoms_read++;
                type0_atoms_read[component]++;
                most_recent_value[component] = result;
                return result;
            }
        } else if (type == 1) {
            // the type [X] i.e., we read how many times the previously read value will repeat
            unsigned int value = data[component][se.byte_idx] & 63; //&31;
            // one repetition used already when we read the value itself:
            st[component].top().atoms_read++;
            repetitions_left[component] = value - 1;
            assert(repetitions_left[component] >= 0);
            st[component].top().byte_idx += 1; // we read one more byte
            // next call will return right after executing the part at the top
            //return getNextValueAndUpdateStack(component);
            repetitions_left[component]--;
            type0_atoms_read[component]++;
            return most_recent_value[component];
        } else if (type == 2) { // case ( offset substring_length ) :
            // read offset in value1:
            unsigned int value1 = data[component][se.byte_idx + 3] & 255;
            value1 |= ((data[component][se.byte_idx + 2] & 255) << 8);
            value1 |= ((data[component][se.byte_idx + 1] & 255) << 16);
            value1 |= (data[component][se.byte_idx] & 1) << 24;
            //value1 |= ((data[component][se.iterator] & 32) << 19);
            assert(value1 < se.byte_idx);
            value1 = se.byte_idx - value1;
            // read length of substring (i.e., # atoms) in value2:
            unsigned int value2 = ((data[component][se.byte_idx] & 255) >> 1) & 31; //15;
            st[component].top().byte_idx += 4; // we've consumed 4 more bytes
            unsigned int mar = min(value2, se.max_atoms_to_read - se.atoms_read);
            st[component].push(StackElement(value1, mar));
            continue;
        } else if (type == 3) { // case (offset substring_length ) with substring_length = 2 or 3
            unsigned int value1 = data[component][se.byte_idx + 2] & 255;
            value1 |= ((data[component][se.byte_idx + 1] & 255) << 8);
            value1 |= ((data[component][se.byte_idx] & 31) << 16);
            assert(value1 <= se.byte_idx);
            value1 = se.byte_idx - value1;
            unsigned int value2 = 2 + ((data[component][se.byte_idx] & 32) >> 5);
            st[component].top().byte_idx += 3; // we've consumed 3 more bytes
            unsigned int mar = min(value2, se.max_atoms_to_read - se.atoms_read);
            st[component].push(StackElement(value1, mar));
            continue;
        } else {
            assert(false);
        }
    }
}
