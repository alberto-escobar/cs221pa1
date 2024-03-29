/**
 *  @file        imglist.cpp
 *  @description Contains partial implementation of ImgList class
 *               for CPSC 221 PA1
 *               Function bodies to be completed by student
 * 
 *  THIS FILE WILL BE SUBMITTED
 */

#include "imglist.h"

#include <math.h> // provides fmax, fmin, and fabs functions

/*********************
* CONSTRUCTORS, ETC. *
*********************/

/**
 * Default constructor. Makes an empty list
 */
ImgList::ImgList() {
    // set appropriate values for all member attributes here
	northwest = NULL;
    southeast = NULL;
}

/**
 * Creates a list from image data
 * @pre img has dimensions of at least 1x1
 */
ImgList::ImgList(PNG& img) {
    // build the linked node structure and set the member attributes appropriately
    
    unsigned width = img.width();
    unsigned height = img.height();
    ImgNode* picture[width][height];

    for (unsigned x = 0; x < width; x++) {
        for (unsigned y = 0; y < height; y++) {
            RGBAPixel * pixel = img.getPixel(x, y);
            ImgNode *node = new ImgNode();
            node->colour = *pixel;
            picture[x][y] = node;
        }
    }

    ImgNode *node;
    for (unsigned y = 1; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            node = picture[x][y];
            node->north = picture[x][y-1];
            node->north->south = node;
        }
    }

    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 1; x < width; x++) {
            node = picture[x][y];
            node->west = picture[x-1][y];
            node->west->east = node;
        }
    }

    northwest = picture[0][0];
    southeast = picture[width - 1][height - 1];
}

/************
* ACCESSORS *
************/

/**
 * Returns the horizontal dimension of this list (counted in nodes)
 * Note that every row will contain the same number of nodes, whether or not
 *   the list has been carved.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionX() const {
    // replace the following line with your implementation
    ImgNode * current = northwest;
    unsigned count = 0;
    while (current != NULL) {
        count++;
        current = current->east;
    }
    return count;
    
}

/**
 * Returns the vertical dimension of the list (counted in nodes)
 * It is useful to know/assume that the grid will never have nodes removed
 *   from the first or last columns. The returned value will thus correspond
 *   to the height of the PNG image from which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   y dimension.
 */
unsigned int ImgList::GetDimensionY() const {
    // replace the following line with your implementation
    ImgNode * current = northwest;
    unsigned count = 0;
    while (current != NULL) {
        count++;
        current = current->south;
    }
    return count;
}

/**
 * Returns the horizontal dimension of the list (counted in original pixels, pre-carving)
 * The returned value will thus correspond to the width of the PNG image from
 *   which this list was constructed.
 * We expect your solution to take linear time in the number of nodes in the
 *   x dimension.
 */
unsigned int ImgList::GetDimensionFullX() const {
    // replace the following line with your implementation
    ImgNode * current = northwest;
    unsigned count = 0;
    while (current != NULL) {
        count = count + 1 + current->skipright;
        current = current->east;
    }
    return count;
}

/**
 * Returns a pointer to the node which best satisfies the specified selection criteria.
 * The first and last nodes in the row cannot be returned.
 * @pre rowstart points to a row with at least 3 physical nodes
 * @pre selectionmode is an integer in the range [0,1]
 * @param rowstart - pointer to the first node in a row
 * @param selectionmode - criterion used for choosing the node to return
 *          0: minimum "brightness" across row, not including extreme left or right nodes
 *          1: node with minimum total of "colour difference" with its left neighbour and with its right neighbour.
 *        In the (likely) case of multiple candidates that best match the criterion,
 *        the left-most node satisfying the criterion (excluding the row's starting node)
 *        will be returned.
 * A note about "brightness" and "colour difference":
 * For PA1, "brightness" will be the sum over the RGB colour channels, multiplied by alpha.
 * "colour difference" between two pixels can be determined
 * using the "distanceTo" function found in RGBAPixel.h.
 */
ImgNode* ImgList::SelectNode(ImgNode* rowstart, int selectionmode) {
    // add your implementation below
    ImgNode* currentNode = rowstart->east;
    ImgNode* selectedNode;
    double minBrightness = 800000;
    double minDifference = 6;
    
    while (currentNode->east != NULL) {
        
        if (selectionmode == 0) {
            double brightness = (currentNode->colour.r + currentNode->colour.g + currentNode->colour.b)*currentNode->colour.a; 
            if (brightness < minBrightness) {
                selectedNode = currentNode;
                minBrightness = brightness;
                
                
            }
        
        } else {
            double difference = currentNode->colour.distanceTo(currentNode->east->colour) + currentNode->colour.distanceTo(currentNode->west->colour);
            if (difference < minDifference) {
                selectedNode = currentNode;
                minDifference = difference;
            }

        }
        currentNode = currentNode->east;
    }
    return selectedNode;
}

/**
 * Renders this list's pixel data to a PNG, with or without filling gaps caused by carving.
 * @pre fillmode is an integer in the range of [0,2]
 * @param fillgaps - whether or not to fill gaps caused by carving
 *          false: render one pixel per node, ignores fillmode
 *          true: render the full width of the original image,
 *                filling in missing nodes using fillmode
 * @param fillmode - specifies how to fill gaps
 *          0: solid, uses the same colour as the node at the left of the gap
 *          1: solid, using the averaged values (all channels) of the nodes at the left and right of the gap
 *          2: linear gradient between the colour (all channels) of the nodes at the left and right of the gap
 *             e.g. a gap of width 1 will be coloured with 1/2 of the difference between the left and right nodes
 *             a gap of width 2 will be coloured with 1/3 and 2/3 of the difference
 *             a gap of width 3 will be coloured with 1/4, 2/4, 3/4 of the difference, etc.
 *             Like fillmode 1, use the smaller difference interval for hue,
 *             and the smaller-valued average for diametric hues
 */
PNG ImgList::Render(bool fillgaps, int fillmode) const {
    // Add/complete your implementation below
    ImgNode * currentRow = northwest;
    int width;
    if (fillgaps) {
        width = GetDimensionFullX();
    } else {
        width = GetDimensionX();
    }
    int height = GetDimensionY();
    PNG outpng = PNG(width, height);
    int x, y;
    y = 0;
    while (y < height && currentRow != NULL) {
        ImgNode * currentColumn = currentRow;
        x = 0;
        while (x < width && currentColumn != NULL) {
            RGBAPixel * pixel = outpng.getPixel(x,y);
            *pixel = currentColumn->colour;
            int skip = 0;
            int skips = currentColumn->skipleft;
            while (skip < skips && fillgaps) {
                
                x++;
                pixel = outpng.getPixel(x,y);
                if (fillmode == 0) {
                    *pixel = RGBAPixel(currentColumn->colour);
                } else if (fillmode == 1) {
                    int avgr = (currentColumn->colour.r + currentColumn->east->colour.r)/2;
                    int avgg = (currentColumn->colour.g + currentColumn->east->colour.g)/2;
                    int avgb = (currentColumn->colour.b + currentColumn->east->colour.b)/2;
                    *pixel = RGBAPixel(avgr, avgg, avgb);
                } else if (fillmode == 2) {
                    int gradr = currentColumn->colour.r + (currentColumn->east->colour.r - currentColumn->colour.r)*(skip+1)/(skips+1);
                    int gradg = currentColumn->colour.g + (currentColumn->east->colour.g - currentColumn->colour.g)*(skip+1)/(skips+1);
                    int gradb = currentColumn->colour.b + (currentColumn->east->colour.b - currentColumn->colour.b)*(skip+1)/(skips+1);
                    *pixel = RGBAPixel(gradr, gradg, gradb);
                }
                skip++;
            }
            x++;
            currentColumn = currentColumn->east;
        }
        currentRow = currentRow->south;
        y++;
    }
  
    return outpng;
}

/************
* MODIFIERS *
************/

/**
 * Removes exactly one node from each row in this list, according to specified criteria.
 * The first and last nodes in any row cannot be carved.
 * @pre this list has at least 3 nodes in each row
 * @pre selectionmode is an integer in the range [0,1]
 * @param selectionmode - see the documentation for the SelectNode function.
 * @param this list has had one node removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(int selectionmode) {
    // add your implementation 

	ImgNode * currentRow = northwest;
    while (currentRow != NULL) {
        
        ImgNode * carvedNode = SelectNode(currentRow, selectionmode);
        
        carvedNode->east->west = carvedNode->west;
        carvedNode->east->skipright += carvedNode->skipright + 1;
        

        carvedNode->west->east = carvedNode->east;
        carvedNode->west->skipleft+= carvedNode->skipleft + 1;
        

        if (carvedNode->north != NULL) {
            carvedNode->north->south = carvedNode->south;
            carvedNode->north->skipdown += carvedNode->skipdown + 1;
            
        }

        if (carvedNode->south != NULL) {
            carvedNode->south->north = carvedNode->north;
            carvedNode->south->skipup+= carvedNode->skipup + 1;
            //
        }
    
        delete carvedNode;
        carvedNode = NULL;
        currentRow = currentRow->south;
    }
}

// note that a node on the boundary will never be selected for removal
/**
 * Removes "rounds" number of nodes (up to a maximum of node width - 2) from each row,
 * based on specific selection criteria.
 * Note that this should remove one node from every row, repeated "rounds" times,
 * and NOT remove "rounds" nodes from one row before processing the next row.
 * @pre selectionmode is an integer in the range [0,1]
 * @param rounds - number of nodes to remove from each row
 *        If rounds exceeds node width - 2, then remove only node width - 2 nodes from each row.
 *        i.e. Ensure that the final list has at least two nodes in each row.
 * @post this list has had "rounds" nodes removed from each row. Neighbours of the created
 *       gaps are linked appropriately, and their skip values are updated to reflect
 *       the size of the gap.
 */
void ImgList::Carve(unsigned int rounds, int selectionmode) {
    // add your implementation here
    unsigned maxRounds = GetDimensionX() - 2;
    if (rounds > maxRounds) {
        rounds = maxRounds;
    }
    int cycle = 0;
    while (cycle < rounds) {
	    Carve(selectionmode);
        cycle++;
    }
}


/*
 * Helper function deallocates all heap memory associated with this list,
 * puts this list into an "empty" state. Don't forget to set your member attributes!
 * @post this list has no currently allocated nor leaking heap memory,
 *       member attributes have values consistent with an empty list.
 */

//ALBERTO CODE
void ImgList::Clear() {
    // add your implementation here
    ImgNode * currentRow = northwest;
    while (currentRow != NULL) {
        ImgNode * currentColumn = currentRow;
        currentRow = currentRow->south;
        while (currentColumn != NULL) {
            ImgNode * temp = currentColumn -> east; 
            delete currentColumn;
            currentColumn = temp;
        }
        
    }
    northwest = NULL;
    southeast = NULL;
}

/**
 * Helper function copies the contents of otherlist and sets this list's attributes appropriately
 * @pre this list is empty
 * @param otherlist - list whose contents will be copied
 * @post this list has contents copied from by physically separate from otherlist
 */
void ImgList::Copy(const ImgList& otherlist) {
    // add your implementation here
    unsigned width = otherlist.GetDimensionX();
    unsigned height = otherlist.GetDimensionY();
	ImgNode * newlistNodes[height][width];
    
    ImgNode * otherlistrow = otherlist.northwest;
    int row = 0;
    int column;
    while (otherlistrow != NULL) {
        ImgNode * otherlistnode = otherlistrow;
        column = 0;
        while (otherlistnode != NULL) {
            newlistNodes[row][column] = new ImgNode(*otherlistnode);
            otherlistnode = otherlistnode->east;
            column++;
        }
        otherlistrow = otherlistrow->south;
        row++;
    }
    ImgNode *node;
    for (unsigned y = 1; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            node = newlistNodes[y][x];
            node->north = newlistNodes[y-1][x];
            node->north->south = node;
        }
    }

    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 1; x < width; x++) {
            node = newlistNodes[y][x];
            node->west = newlistNodes[y][x-1];
            node->west->east = node;
        }
    }
    
    northwest = newlistNodes[0][0];
    southeast = newlistNodes[height - 1][width - 1];
}

/*************************************************************************************************
* IF YOU DEFINED YOUR OWN PRIVATE FUNCTIONS IN imglist-private.h, YOU MAY ADD YOUR IMPLEMENTATIONS BELOW *
*************************************************************************************************/

