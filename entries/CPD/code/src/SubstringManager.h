/* 
 * File:   SubstringManager.h
 * Author: abotea
 *
 * Created on 27 May 2011, 5:41 PM
 */

#ifndef _SUBSTRINGMANAGER_H
#define	_SUBSTRINGMANAGER_H

#include "CompressedTable.h"
#include "Rectangle.h"

class SubstringManager {
public:
    SubstringManager();
    SubstringManager(const SubstringManager& orig);
    virtual ~SubstringManager();
    void addToCountingTrees(const CompressedTable & ct);
private:
    void addSublistToCountingTree(int start_idx);
};

#endif	/* _SUBSTRINGMANAGER_H */

