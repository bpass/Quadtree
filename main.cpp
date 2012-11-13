#include "main.h"
#include "assert.h"

/***********************************************************
 *
 *
 * MAIN FUNCTIONS
 *
 *
 ***********************************************************/

/**
 * \brief This is the main function that creates a quadtree.
 */
int main(){

    string 			inimgfn = "subset_3_utm2.img";
    string 			outimgreclassfn = "reclassified_image1.img";
    string 			outimgprunefn = "reclassified_image2.img";
    string 			originalnodefn = "original_nodes.txt";
    string 			reclassnodefn = "reclass_nodes.txt";

    Quadtree*     	original_tree;
    Quadtree*    	reclass_tree;
    float**     	raster_data;
    int         	xsize=0;
    int         	ysize=0;

    emptyValue = 0;

    initialize();

     /* Original tree part */
     cerr << "Opening image...\n";
     GDALDataset* dataIn = (GDALDataset *) GDALOpen( inimgfn.c_str(), GA_ReadOnly );
     if(!dataIn){
         cerr << "Error: Opening image \"" << inimgfn.c_str() << "\", leaving program\n";
         return 0;
     }

     xsize = dataIn->GetRasterXSize();
     ysize = dataIn->GetRasterYSize();

     fprintf(stderr,"Reading image...\n");
     raster_data = readImage(dataIn);
     assert(raster_data!=NULL);

     fprintf(stderr,"Outputting image read as testimg1.img...\n");
     createImage(dataIn,raster_data,xsize,ysize,"testimg1.img");

     fprintf(stderr,"Building tree...\n");
     original_tree = Quadtree::constructTree(raster_data,xsize,ysize);
     assert(original_tree!=NULL);

     fprintf(stderr, "Verifying tree structure...\n");
     assert(original_tree->VerifyTree()==NO_ERROR);

     fprintf(stderr,"Verifying tree covers entire image...\n");
     assert(original_tree->VerifyCoverage());

     fprintf(stderr,"Original Nodes: %d\n",original_tree->NodeCount());
     fprintf(stderr,"Original Leaves: %d\n",original_tree->LeafCount());

     fprintf(stderr,"Saving node information to %s...\n",originalnodefn.c_str());
     original_tree->SaveNodeInfo(originalnodefn);

     fprintf(stderr,"Rebuilding image from the tree...\n");
     float** newimg2 = original_tree->RebuildImage();

     fprintf(stderr,"Outputting image created as testimg2.img...\n");
     createImage(dataIn,newimg2,xsize,ysize,"testimg2.img");

     fprintf(stderr,"Pruning tree...\n");
     original_tree->Prune();

     fprintf(stderr,"Pruned Nodes: %d\n",original_tree->NodeCount());
     fprintf(stderr,"Pruned Leaves: %d\n",original_tree->LeafCount());

     fprintf(stderr,"Updating node count...\n");
     original_tree->Update();

     fprintf(stderr,"Pruned Nodes: %d\n",original_tree->NodeCount());
     fprintf(stderr,"Pruned Leaves: %d\n",original_tree->LeafCount());

     fprintf(stderr,"Rebuilding Image...\n");
     float** newimg = original_tree->RebuildImage();

     fprintf(stderr,"Creating Image...\n");
     createImage(dataIn,newimg,xsize,ysize,"testimg_withprune.img");

     fprintf(stderr,"Reading node file...\n");
     exit(1);
     float** original_nodes = readNodeFile(originalnodefn);


     /* Reclassified tree part */
     cerr << "Reclassifying data... \n";
     //float** reclass_data = reclassify(raster_data,xsize,ysize);

     cerr << "Reclassifying image... \n";
     original_nodes = reclassify(raster_data,xsize,ysize);

     cerr << "Creating reclassified quadtree... \n";
     reclass_tree = Quadtree::constructTree(original_nodes,xsize,ysize);

     cerr << "Saving reclassified quadtree... \n";
     reclass_tree->SaveNodeInfo(reclassnodefn);

     cerr << "Counting reclass nodes...\n" <<
             reclass_tree->NodeCount() << " nodes ";
     cerr << "Counting reclass leaves...\n" <<
             reclass_tree->LeafCount() << " leaves ";

     cerr << "Creating GDAL image of reclassified data... \n";
     createImage(dataIn,original_nodes,xsize,ysize,"reclassified_image.img");

     cerr << "Creating DOT files...";
 /*
     if(DrawTree(fullTree) != NO_ERROR)
     {
         cerr << "ERROR DRAWING TREE\n";
         return 0;
     }
 */
     cerr << "done\nSize decreased by "
             << original_tree->NodeCount() - reclass_tree->NodeCount() << endl;

     cerr << "done\nExiting\n";

     return 1;
}

/***********************************************************
 *
 *
 * FUNCTIONS
 *
 *
 ***********************************************************/

void initialize(){

    GDALAllRegister();
    /*
    cout << "Enter the name of the input file: ";
    cin >> ifname;
    cout << "Enter the name of the output file: ";
    cin >> ofname;
    cout << "Enter output format: ";
    cin >> outformat;
    */
    //GDALDriver* test =
}

void print2DImage(float** img,int w,int h){
    int i,j;
    cerr << "Image Size: " << w << "," << h << endl;
    for(i=0;i<w;i++){
        for(j=0;j<h;j++)
            cerr << img[i][j] << " ";
        cerr << endl;
    }
}

float** readNodeFile(string filename){
    ifstream in;
    in.open(filename.c_str());
    if(in.fail()){
        cerr << "Failed to open input file \"" << filename << "\"\n";
        return NULL;
    }

    string r;
    int i,j,x,y,numNodes,xsize,ysize;


    getline(in,r);
    istringstream row(r);
    if(r!="NODEFILE"){
        cerr << "Error: File \"" << filename << "\" is not a nodefile.\n";
        cerr << "Got \"" << r << "\", Expected \"NODEFILE\"\n";
        return NULL;
    }

    getline(in,r);
    istringstream row2(r);
    row2 >> numNodes >> xsize >> ysize;

    Node nodeArr[numNodes];
    int** nodes = new int*[numNodes];
    for(i=0;i<numNodes;i++)
        nodes[i]=new int[6];

    float** img = new float*[xsize];
    for(i=0;i<xsize;i++)
        img[i]=new float[ysize];

    for(i=0;i<numNodes;i++)
    {
        getline(in,r);
        stringstream row(r);
        for(j=0;j<6;j++)
            row >> nodes[i][j];

        Node a = {nodes[i][0],nodes[i][1],nodes[i][2],
                nodes[i][3],nodes[i][4],nodes[i][5]};
        nodeArr[i] = a;

        for(x=nodes[i][0];x<nodes[i][0]+nodes[i][2];x++)
            for(y=nodes[i][1];y<nodes[i][1]+nodes[i][3];y++)
                if(nodes[i][4]!=emptyValue) img[x][y]=nodes[i][4];
    }

    sortNodes(nodeArr,0,numNodes-1);

    int nlevels = nodeArr[numNodes-1].level;

    for(j=0;j<=nlevels;j++){
        if(VERBOSE) cout << "Level " << j << ":  ";
        for(i=0;i<numNodes;i++)
            if(nodeArr[i].level==j)
                if(VERBOSE) cout << setw(5) << nodeArr[i].value;
        if(VERBOSE) cout << endl;
    }

    for(i=0;i<numNodes;i++) delete [] nodes[i];
    delete [] nodes;

    return img;

}

float** readImage(GDALDataset* gdalData){

    int i,j,xsize,ysize;
    float *pafScanline;
    float** myData;

    GDALRasterBand* gdalBand = gdalData->GetRasterBand(1);
    if( !gdalBand ){
        cerr << "Error fetching raster band\n";
        return NULL;
    }

    xsize = gdalBand->GetXSize();
    ysize = gdalBand->GetYSize();

    myData = new float*[xsize];
    for(i=0;i<xsize;i++) myData[i] = new float[ysize];

    pafScanline = new float[xsize*ysize];

    for(i=0;i<xsize;i++)
        for(j=0;j<ysize;j++)
            myData[i][j] = emptyValue;

    gdalBand->RasterIO(GF_Read,0,0,xsize,ysize,
            pafScanline,xsize,ysize,GDT_Float32,0,0);

    for(j=0;j<ysize;++j)
        for(int k=0;k<xsize;++k)
            myData[k][j] = pafScanline[j*xsize+k];

    delete [] pafScanline;

    return myData;
}

QT_ERR createImage(GDALDataset* poSrcDS, float** data,int w,int h,string fname){

    GDALDriver *gdalDriver = GetGDALDriverManager()->GetDriverByName(outformat);
    if( gdalDriver == NULL ) return READ_ERROR;

    char **gdalMetadata = gdalDriver->GetMetadata();

    if( !CSLFetchBoolean( gdalMetadata, GDAL_DCAP_CREATECOPY, FALSE ) ){
        printf( "Driver %s does not support Create() method."
                "\nCannot create image.\n", outformat );
        return WRITE_ERROR;
    }

    GDALDataset *poDstDS = gdalDriver->CreateCopy(
                                    fname.c_str(),
                                    poSrcDS,
                                    FALSE,
                                    NULL,
                                    NULL,
                                    NULL
                                    );

    GDALRasterBand* band = poDstDS->GetRasterBand(1);

    for (int i=0; i<w; ++i) {
            band->RasterIO(
            GF_Write,
            i,0,
            1,h,
            data[i],
            1,h,
            GDT_Float32,
            0,0);
    }

    GDALClose( (GDALDatasetH) poDstDS );

    return NO_ERROR;
}

QT_ERR dumpToFile(float** img,int w, int h, string fname){
    ofstream out;
    out.open(fname.c_str(),ios::out);
    if(out.fail()) return WRITE_ERROR;

    for(int i=0;i<w;i++){
        for(int j=0;j<h;j++){
            out << img[i][j] << " ";
        }
        out << endl;
    }

    return NO_ERROR;
}

float** reclassify(float** img,int w, int h){
    for(int i=0;i<w;i++){
        for(int j=0;j<h;j++){
            if(img[i][j]<=20 || img[i][j]>24)
                img[i][j]=4;
            else if(img[i][j]==21)
                img[i][j]=3;
            else if(img[i][j]==22)
                img[i][j]=2;
            else
                img[i][j]=1;
        }
    }

    return img;
}

void printNodeInfo(Node n){
    cout << "Node:\n\t X: " << n.x
            << " Y: " << n.y
            << " W: " << n.width
            << " H: " << n.height
            << " Value: " << n.value
            << " Level: " << n.level
            << endl;
}

void printGDALDatasetInfo(GDALDataset* gdalData){

    double originalGeoTransform[6];
    printf( "Driver: %s/%s\n",
            gdalData->GetDriver()->GetDescription(),
            gdalData->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

    printf( "Size is %dx%dx%d\n",
            gdalData->GetRasterXSize(), gdalData->GetRasterYSize(),
            gdalData->GetRasterCount() );

    if( gdalData->GetProjectionRef()  != NULL )
        printf( "Projection is `%s'\n", gdalData->GetProjectionRef() );

    if( gdalData->GetGeoTransform( originalGeoTransform ) == CE_None )
    {
        printf( "Origin = (%.6f,%.6f)\n",
                originalGeoTransform[0], originalGeoTransform[3] );

        printf( "Pixel Size = (%.6f,%.6f)\n",
                originalGeoTransform[1], originalGeoTransform[5] );
    }
}

void printGDALRasterInfo(GDALRasterBand* poBand){
    int             nBlockXSize, nBlockYSize;
    int             bGotMin, bGotMax;
    double          adfMinMax[2];

    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
    printf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
           nBlockXSize, nBlockYSize,
           GDALGetDataTypeName(poBand->GetRasterDataType()),
           GDALGetColorInterpretationName(
                                          poBand->GetColorInterpretation()) );

    adfMinMax[0] = poBand->GetMinimum( &bGotMin );
    adfMinMax[1] = poBand->GetMaximum( &bGotMax );
    if( ! (bGotMin && bGotMax) )
        GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);

    printf( "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );

    if( poBand->GetOverviewCount() > 0 )
        printf( "Band has %d overviews.\n", poBand->GetOverviewCount() );

    if( poBand->GetColorTable() != NULL )
        printf( "Band has a color table with %d entries.\n",
               poBand->GetColorTable()->GetColorEntryCount() );
}

void sortNodes(Node arr[], int left, int right) {
      int i = left, j = right;
      Node tmp;
      int pivot = arr[(left + right) / 2].level;
      while (i <= j) {
            while (arr[i].level < pivot) i++;
            while (arr[j].level > pivot) j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++; j--;
            }
      };

      if (left < j) sortNodes(arr, left, j);
      if (i < right) sortNodes(arr, i, right);
}

QT_ERR DrawTree(Quadtree* tree){

    ofstream out;
    out.open("graph.dot",ios::out);

    if(out.fail()){
        return WRITE_ERROR;
    }
    out << "graph reclassified_tree.gv {" << endl;
    if(tree->DrawTree(out) != NO_ERROR){
        cerr << "Error drawing tree\n";
        return FILE_ERROR;
    }
    out << "}";
    out.close();

    return NO_ERROR;

}
