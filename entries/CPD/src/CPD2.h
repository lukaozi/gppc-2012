/* 
 * File:   CPD2.h
 * Author: abotea
 *
 * Created on 29 August 2011, 11:49 AM
 */

#ifndef CPD2_H
#define	CPD2_H

#include "AbstractCPD.h"
#include <assert.h>
#include <stack>

class CPD2 : public AbstractCPD {
public:
    CPD2();
    CPD2(const CPD2& orig);
    virtual ~CPD2();    
    void readDataFromStream2(ifstream & infile);
    int getMove(int startIdx, int targetIdx, int & nr_rect_checks,
        const AbstractGraph * graph);
private:
    struct StackElement {
        StackElement()
        {}
        StackElement(unsigned int bi, unsigned int mar)
                :byte_idx(bi), max_atoms_to_read(mar), atoms_read(0)
        {}
        // byte index in a string (e.g., left-column string) where to look next
        unsigned int byte_idx;
        // max number of atoms that we are allowed to read 
        // (initial value given from element below in the stack)
        unsigned int max_atoms_to_read;
        unsigned int atoms_read;
    };
    struct FastStack {
        FastStack() 
        :topidx(-1)
        {};
        void push(const StackElement & se) {
            //assert (topidx < 1000);
            topidx++;
            data[topidx] = se;
        }
        inline StackElement & top() {
            //assert (topidx >= 0);
            return data[topidx];
        } 
        void pop() {
            //assert (topidx >= 0);
            topidx--;
        }
        bool empty() {
            return topidx == -1;
        }
        void clear() {
            topidx = -1;
        }
        StackElement data[1000];
        int topidx;
    };

    int getDefaultMove(int stIdx, int tgIdx, const AbstractGraph *gr) const;    
    int getHead(int nodeIdx, int component, int quarter) const {
        assert (0 <= component && component <= 4);
        assert (0 <= nodeIdx && nodeIdx <= nr_nodes);
        return heads[component][4*nodeIdx+quarter];
    }
    
    unsigned int getTypeOfRecord(int component) {
        return (data[component][st[component].top().byte_idx] & 255) >> 6;
    }
    
    void initComponentStack(int nodeIdx, int component, int quarter) {
        unsigned int head = getHead(nodeIdx, component, quarter);
        // TODO: handle case when nodeIdx is the last index; 
        // now sure if that's a problem any longer
        unsigned int length = list_length[4*nodeIdx+quarter];
        st[component].clear();
        st[component].push(StackElement(head, length));
        repetitions_left[component] = 0;
        type0_atoms_read[component] = 0;
    }
    
    unsigned int getNextValueAndUpdateStack(int component);
    unsigned int getNextValueAndUpdateStack2(int component);
    
    // data as 5 blocks of bytes
    char* data[5];
    // the starting position of the sub-block corresponding
    // to a given node (identified by its global index);
    // as a convention, heads[k][nr_nodes] stores the length of the entire k-th block in bytes
    int* heads[5];
    int nr_heads[5];
    int block_size[5];
    // stack with pointers to various places in the block
    FastStack st[5];
    int most_recent_value[5];
    int repetitions_left[5];
    int nr_nodes; // the number of nodes in the graph
    char *default_moves;
    int *list_length; // # rectangles in a list
    int type0_atoms_read[5];
};

#endif	/* CPD2_H */

