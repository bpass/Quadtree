/**
 * \file Quadtree.h
 * \brief This file defines the structure of a quadtree
 * \date Jul 10, 2012
 * \author bpassuello
 * \copyright USGS
 */

#ifndef QUADTREE_H_
#define QUADTREE_H_

#include "Utility.h"

using namespace std;

/**
 * \brief
 *         A struct used to hold data about a certain node in the tree.
 */
struct Node{
    int     x; /**< The x value of the node */
    int     y; /**< The y value of the node */
    int     width; /**< The width of the area specified by the node */
    int     height; /**< The height of the area specified by the node */
    float   value; /**< The thematic value of the are specified by the node */
    int     level; /**< The level of the quadtree in which the node appears */
};

/**
 * \class Quadtree Quadtree.h "Quadtree.h"
 * \brief This class holds the structure of a single node of a quadtree.
 */
class Quadtree {

    public:

        static Quadtree* constructTree(float**,int,int);

        bool    SaveToFile(string fname); /**< Prints the tree */
        int     NodeCount(); /**< Counts the total amount of nodes in the tree */
        int     LeafCount(); /**< Counts the amount of leaf nodes in the tree */
        int     Value();
        void    Update();
        void    PrintTreeInfo();
        float** RebuildImage();
        QT_ERR  DrawTree(ofstream&);
        QT_ERR  Prune();

        ~Quadtree();

    protected:

        int     x;          /**< Holds the x coordinate of the root of the tree */
        int     y;          /**< Holds the y coordinate of the root of the tree */
        int     width;      /**< Holds the width of the node at the root */
        int     height;     /**< Holds the height of the node at the root */
        int     value;      /**< Holds the theme value if the node is homogeneous */
        int     level;      /**< Holds the level of the quadtree that the root is at */
        int     maxlevel;   /**< Holds the maximum level number of the quadtree */
        int     nodes;      /**< Holds the number of nodes in the tree */
        int     leaves;     /**< Holds the number of leaves in the tree */
        string  name;       /**< Holds the name of the node to be used for output */

        int     UpdateNodes();
        int     UpdateLeaves();
        float** RebuildImage(float**);
        float** VerifyCoverage(float**);

    private:

        Quadtree(Quadtree*, float**,int,int,int,int,int,int); /**< Constructor that will be used in the building of the tree */

        void SaveSubtree(string fname); /**< Helper method */

        Quadtree* NW;     /**< Holds the northwest child */
        Quadtree* SW;     /**< Holds the southwest child */
        Quadtree* SE;     /**< Holds the southeast child */
        Quadtree* NE;     /**< Holds the northeast child */
        Quadtree* Parent;/**< Holds the parent node */
};

#endif /* QUADTREE_H_ */
