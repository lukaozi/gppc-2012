/* 
 * File:   utils.h
 * Author: Abotea
 *
 * Created on April 13, 2010, 11:14 AM
 */

#ifndef _UTILS_H
#define	_UTILS_H

#include <iostream>
#include "CompressedMoveDatabase.h"
#include "CompressedTable.h"
#include "GenericNode.h"
#include "SlimmerRectangle.h"
#include "AbstractGraphNode.h"

ostream & operator << (ostream &os, const CompressedMoveDatabase & cmd);
ostream & operator << (ostream &os, const CompressedTable & ct);
ostream & operator << (ostream &os, const Rectangle & rectangle);
ostream & operator << (ostream &os, const SlimRectangle & rectangle);
ostream & operator << (ostream &os, const SlimmerRectangle & rectangle);
ostream & operator << (ostream &os, const GenericNode & node);
ostream & operator << (ostream &os, const GenericEdge & edge);

bool comp(AbstractGraphNode* n1, AbstractGraphNode* n2);

#endif	/* _UTILS_H */

