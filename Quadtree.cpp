/**
 * \file Quadtree.cpp
 * \brief This is the implementation of the Quadtree class.
 * \date Jul 10, 2012
 * \author bpassuello
 * \update Oct 25, 2012
 * \copyright USGS
 */

#include "Quadtree.h"

using namespace std;

/****************************************
 * 				CONSTRUCTORS
 ****************************************/

Quadtree::Quadtree(Quadtree* p,
                        float** mdata,
                        int mx,
                        int my,
                        int mwidth,
                        int mheight,
                        int mlevel,
                        int mmaxlevel){

    if( mwidth<=0 || mheight<=0 || mx < 0 || my < 0 ){
        fprintf(stderr,"Error: Out of bounds dimensions in Quadtree()\n\
        		X:%d Y:%d W:%d H:%d\n",mx,my,mwidth,mheight);
        return;
    }

    x 			= 	mx;
    y 			= 	my;
    width 		= 	mwidth;
    height 		= 	mheight;
    value 		= 	emptyValue;
    level 		= 	mlevel;
    maxlevel 	= 	mmaxlevel;
    nodes=leaves = 1;
    NW=NE=SW=SE=NULL;
    Parent = p;

    if(level==maxlevel)
    {
        value = mdata[x][y];
        return;
    }

    int i,j;
    float color=mdata[x][y];
    bool homo = true;
    for(i=x;i<x+width;i++)
    {
        for(j=y;j<y+height;j++)
        {
            if(mdata[i][j]!=color)
            {
                homo = false;
                break;
            }
        }
        if(!homo) break;
    }

    if(homo){
        value = color;
        return;
    }

    int halfwidth = width/2;
    int halfheight = height/2;

    if(width <2){								/* Width is 1 */

		NW = new Quadtree(this, mdata, x, y,
				1,halfheight, level+1, maxlevel );

		SW = new Quadtree(this, mdata, x, y + halfheight,
				1,height - halfheight, level+1, maxlevel );

        nodes += NW->nodes + SW->nodes;
        leaves = NW->leaves + SW->leaves;
    }
    else if(height < 2){						/* Height is 1 */

		NW = new Quadtree(this, mdata, x, y,
				halfwidth,1, level+1, maxlevel );

		NE = new Quadtree(this, mdata, x+halfwidth, y,
				width - halfwidth,1, level+1, maxlevel );

        nodes += NW->nodes + NE->nodes;
        leaves = NW->leaves + NE->leaves;
    }
    else{

		NW = new Quadtree(this, mdata, x, y,
				halfwidth,halfheight, level+1, maxlevel );

		NE = new Quadtree(this, mdata, x + halfwidth, y,
				width - halfwidth,halfheight, level+1, maxlevel );

		SW = new Quadtree(this, mdata, x, y + halfheight,
				halfwidth,height - halfheight, level+1, maxlevel );

		SE = new Quadtree(this, mdata, x + halfwidth, y + halfheight,
				width - halfwidth,height - halfheight, level+1, maxlevel );

	    nodes += NW->nodes + NE->nodes + SW->nodes + SE->nodes;
	    leaves = NW->leaves + NE->leaves + SW->leaves + SE->leaves;

    }
    /* Naming stuff for graphviz
    char buffer[64];
    sprintf(buffer,"%d%d%d%d",x,y,width,height);
    name.append("node");
    name.append(buffer);
	*/
}

Quadtree* Quadtree::constructTree(float** data,int width,int height){

    int nlevels=0;
    while(pow(2.0,nlevels) < max(width,height))
        nlevels++;

    return new Quadtree(NULL,data,0,0,width,height,0,nlevels);
}

/****************************************
 * 				METHODS
 ****************************************/

/*
 * X Y Width Height Value
 */
bool Quadtree::SaveToFile(string fname){

    remove(fname.c_str());
    ofstream outFile;
    outFile.open(fname.c_str(),ios::app);
    if(outFile.fail()){
        cerr << "Failed to open output file \"" << fname << "\"\n";
        return false;
    }

    outFile << "NODEFILE" << endl;
    outFile <<  leaves << " " <<
                width << " " <<
                height << " " <<
                endl;

    outFile.close();

    NW->SaveSubtree(fname);
    SW->SaveSubtree(fname);
    SE->SaveSubtree(fname);
    NE->SaveSubtree(fname);

    return true;
}

void Quadtree::SaveSubtree(string fname){

	if(value != emptyValue){
		ofstream outFile;
		outFile.open(fname.c_str(),ios::app);
		if(outFile.fail()){
			cerr << "Failed to open output file\n";
			return;
		}
		outFile << x << " " <<
				   y << " " <<
				   width << " " <<
				   height << " " <<
				   value  <<
				   endl;
		outFile.close();
	}

    if(NW) NW->SaveSubtree(fname);
    if(SW) SW->SaveSubtree(fname);
    if(SE) SE->SaveSubtree(fname);
    if(NE) NE->SaveSubtree(fname);

}

QT_ERR Quadtree::Prune(){

    /* This part is for leaves */
    if(value != emptyValue){
        if(NW || SW || NE || SE){
            cerr << "Error: Not all children are null in Prune()\n";
            exit(1);
        }
        if(value<21 || value>24) value = 4;
        else if(value==21) value = 3;
        else if(value==22) value = 2;
        else value = 1;

        nodes=1;
        leaves=1;
    }
    else{

		/* This part is for interior nodes */
		vector<Quadtree*> children;
		if(NW) children.push_back(NW);
		if(SW) children.push_back(SW);
		if(NE) children.push_back(NE);
		if(SE) children.push_back(SE);

		for(uint i = 0; i < children.size(); i++){
			if( children[i]->Prune() != NO_ERROR )
				return TREE_ERROR;
		}

		int val = children.front()->value;
		for(uint i = 0; i < children.size(); i++){
			if(children[i]->value != val){
				return NO_ERROR;
			}
		}
		if(val==emptyValue)
			return NO_ERROR;

		value = children.front()->value;
		NW=NE=SW=SE=NULL;
		//leaves -= (children.size() - 1);
		//nodes -= children.size();
    }
    //fprintf(stderr,"New leaves: %d New nodes: %d\n",leaves,nodes);
    return NO_ERROR;
}

void Quadtree::Update(){
    UpdateNodes();
    UpdateLeaves();
}

int Quadtree::UpdateNodes(){
	nodes=1;
	if(NW) nodes += NW->UpdateNodes();
	if(NE) nodes += NE->UpdateNodes();
	if(SW) nodes += SW->UpdateNodes();
	if(SE) nodes += SE->UpdateNodes();
    return nodes;
}

int Quadtree::UpdateLeaves(){
    if(value!=emptyValue){
        leaves=1;
    }
    else{
        leaves=0;
        if(NW) leaves += NW->UpdateLeaves();
        if(NE) leaves += NE->UpdateLeaves();
        if(SW) leaves += SW->UpdateLeaves();
        if(SE) leaves += SE->UpdateLeaves();
    }
    return leaves;
}

float** Quadtree::RebuildImage(){
    float** img = new float*[width];
    for(int i=0;i<width;i++){
        img[i] = new float[height];
    }

    return RebuildImage(img);
}

float** Quadtree::RebuildImage(float** img){
    if(value==emptyValue){
    	if(NW) img = NW->RebuildImage(img);
    	if(NE) img = NE->RebuildImage(img);
    	if(SW) img = SW->RebuildImage(img);
    	if(SE) img = SE->RebuildImage(img);
    }
    else{
        for(int i=x;i<x+width;i++)
            for(int j=y;j<y+height;j++)
                img[i][j]=value;
    }
    return img;
}

QT_ERR Quadtree::DrawTree(ofstream &out){

    if(out.fail()){
        cerr << "Stream error\n";
        return WRITE_ERROR;
    }

    if(Parent)
    out << name << " -- " << Parent->name << endl;

    if(value==emptyValue){
        NW->DrawTree(out);
        NE->DrawTree(out);
        SW->DrawTree(out);
        SE->DrawTree(out);
    }

    return NO_ERROR;
}

void Quadtree::PrintTreeInfo(){
    cerr << "X: " << x << endl <<
            "Y: " << y << endl <<
            "W: " << width << endl <<
            "H: " << height << endl <<
            "Value: " << value << endl <<
            "Level: " << level << endl <<
            "Max Level: " << maxlevel << endl;
}

/****************************************
 * 				ACCESSORS
 ****************************************/

int Quadtree::NodeCount(){
    return nodes;
}

int Quadtree::LeafCount(){
    return leaves;
}

int Quadtree::Value(){
    return value;
}


/*********************************
 * 				DESTRUCTOR
 *********************************/

Quadtree::~Quadtree() {
    if(NW!=NULL) delete NW;
    if(NE!=NULL) delete NE;
    if(SE!=NULL) delete SE;
    if(SW!=NULL) delete SW;
}
