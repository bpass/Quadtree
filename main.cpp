#include "main.h"

/***********************************************************
 *
 *
 * MAIN FUNCTIONS
 *
 *
 ***********************************************************/


	string			inimgfn = "Images\/nlcd_2006.img";
	//string 			inimgfn = "Images\/subset_3_utm2.img";
    string 			outimgreclassfn = "Results\/reclassified_image1.img";
    string 			outimgprunefn = "Results\/reclassified_image2.img";
    string 			originalnodefn = "Results\/original_nodes.txt";
    string 			reclassnodefn = "Results\/reclass_nodes.txt";
    GDALDataset* 	dataIn;
    GDALColorTable*	colorTable;
    Quadtree*     	original_tree;
    Quadtree*    	reclass_tree;
    float**     	raster_data;
    int         	xsize = 0;
    int         	ysize = 0;

    /* TIMING STUFF */
    timeval stop, start;
    long times[] = {0,0,0,0,0,0};
    long temp = 0;

/**
 * \brief This is the main function that creates a quadtree.
 */
int main(){

	emptyValue = 0;

	/* Original tree part */
	fprintf(stderr,"Opening image from file \"%s\"...\n",inimgfn.c_str());
	initialize();
	colorTable = dataIn->GetRasterBand(1)->GetColorTable();
	fprintf(stderr,"Reading image...\n");
	gettimeofday(&start, NULL);
	raster_data = readImage(dataIn);
	gettimeofday(&stop, NULL);
	temp = stop.tv_usec - start.tv_usec;
	if(temp < 0) temp = 1000000 - temp;
	times[0] += temp;
	fprintf(stderr,"Reading took %i microseconds\n",temp);
	assert(raster_data!=NULL);

	PRINTLINE();
	fprintf(stderr,"Outputting image read as Results\/testimg1.img...\n");
	PRINTLINE();
	createImage(dataIn,raster_data,xsize,ysize,"Results\/testimg1.img");

	fprintf(stderr,"Building tree...\n");
	gettimeofday(&start, NULL);
	original_tree = Quadtree::constructTree(raster_data,xsize,ysize);
	gettimeofday(&stop, NULL);

	temp = stop.tv_usec - start.tv_usec;
	if(temp < 0) temp = 1000000 - temp;
	times[1]+=temp;
	fprintf(stderr,"constructing took %i microseconds\n",temp);
	assert(original_tree!=NULL);

	PRINTLINE();
	fprintf(stderr,"Original Nodes: %d\n",original_tree->NodeCount());
	fprintf(stderr,"Original Leaves: %d\n",original_tree->LeafCount());
	PRINTLINE();

	fprintf(stderr,"Saving node information to %s...\n",originalnodefn.c_str());
	original_tree->SaveNodeInfo(originalnodefn);

	fprintf(stderr,"Rebuilding image from the tree...\n");
	float** newimg2 = original_tree->RebuildImage();

	PRINTLINE();
	fprintf(stderr,"Outputting image created as Results\/testimg2.img...\n");
	PRINTLINE();
	createImage(dataIn,newimg2,xsize,ysize,"Results\/testimg2.img");

	fprintf(stderr,"Pruning tree...\n");
	gettimeofday(&start, NULL);
	original_tree->Prune();
	gettimeofday(&stop, NULL);

	temp = stop.tv_usec - start.tv_usec;
	  if(temp < 0) temp = 1000000 - temp;
	times[2]+= temp;
	fprintf(stderr,"pruning took %i microseconds\n",temp);

	PRINTLINE();
	fprintf(stderr,"Saving pruned node info to Results\/pruned_nodes.txt\n");
	PRINTLINE();
	original_tree->SaveNodeInfo("Results\/pruned_nodes.txt");

	fprintf(stderr,"Updating node count...\n");
	original_tree->Update();

	fprintf(stderr,"Pruned Nodes: %d\n",original_tree->NodeCount());
	fprintf(stderr,"Pruned Leaves: %d\n",original_tree->LeafCount());

	fprintf(stderr,"Rebuilding image from quadtree...\n");

	gettimeofday(&start,NULL);
	float** newimg = original_tree->RebuildImage();
	gettimeofday(&stop, NULL);

	temp = stop.tv_usec - start.tv_usec;
	  if(temp < 0) temp = 1000000 - temp;
	times[3]+=temp;
	fprintf(stderr,"Rebuilding took %i microseconds\n",temp);

	PRINTLINE();
	fprintf(stderr,"Creating rebuilt image as Results\/testimg3.img...\n");
	PRINTLINE();
	gettimeofday(&start,NULL);
	createImage(dataIn,newimg,xsize,ysize,"Results\/testimg3.img");
	gettimeofday(&stop, NULL);

	temp = stop.tv_usec - start.tv_usec;
	if(temp < 0) temp = 1000000 - temp;
	times[4]+=temp;
	fprintf(stderr,"output took %i microseconds\n",temp);

	/* Reclassified tree part */
	fprintf(stderr,"Reading node file \"%s\"...\n",originalnodefn.c_str());
	float** original_nodes = readNodeFile(originalnodefn);

	fprintf(stderr,"Creating reclassified quadtree... \n");
	reclass_tree = Quadtree::constructTree(original_nodes,xsize,ysize);

	fprintf(stderr,"Creating GDAL image of reclassified data to %s... \n",outimgreclassfn.c_str());
	createImage(dataIn,original_nodes,xsize,ysize,outimgreclassfn);

	fprintf(stderr,"Done\n");
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

    dataIn = (GDALDataset *) GDALOpen( inimgfn.c_str(), GA_ReadOnly );
    if(!dataIn){
        cerr << "Error: Opening image \"" << inimgfn.c_str() << "\", leaving program\n";
        exit(1);
    }

    xsize = dataIn->GetRasterXSize();
    ysize = dataIn->GetRasterYSize();
}

/*
 * X Y Width Height Value Level
 */
float** readNodeFile(string filename){
    ifstream in;
    in.open(filename.c_str());
    if(in.fail()){
    	fprintf(stderr, "Failed to open input file\"%s\"\n",filename.c_str());
        return NULL;
    }

    string r;
    int i,j,x,y,numNodes,xsizelocal,ysizelocal;

    getline(in,r);
    istringstream row(r);
    if(r!="NODEFILE"){
        cerr << "Error: File \"" << filename << "\" is not a nodefile.\n";
        cerr << "Got \"" << r << "\", Expected \"NODEFILE\"\n";
        return NULL;
    }

    getline(in,r);
    istringstream row2(r);
    row2 >> numNodes >> xsizelocal >> ysizelocal;

    fprintf(stderr,"Nodes: %d X: %d Y: %d\n",numNodes,xsizelocal,ysizelocal);

    fprintf(stderr,"Allocating array\n");
    Node nodeArr[numNodes];
    int** nodes = new int*[numNodes];
    for(i=0;i<numNodes;i++)
        nodes[i]=new int[5];

    fprintf(stderr,"Completed allocating the img array\n");

    float** img = new float*[xsizelocal];
    for(i=0;i<xsizelocal;i++)
        img[i]=new float[ysizelocal];

    fprintf(stderr,"About to enter for loop\n");
    for(i=0;i<numNodes;i++)
    {
        getline(in,r);
        stringstream row(r);
        for(j=0;j<5;j++)
            row >> nodes[i][j];

        Node a = {nodes[i][0],nodes[i][1],nodes[i][2],
                nodes[i][3],nodes[i][4],nodes[i][5]};
        nodeArr[i] = a;

        for(x=nodes[i][0];x<nodes[i][0]+nodes[i][2];x++)
            for(y=nodes[i][1];y<nodes[i][1]+nodes[i][3];y++)
                img[x][y]=nodes[i][4];
    }

    for(i=0;i<numNodes;i++) delete [] nodes[i];
    delete [] nodes;

    return img;

}

float** readImage(GDALDataset* gdalData){

    int i,j,x,y;
    float *pafScanline;
    float** myData;

    GDALRasterBand* gdalBand = gdalData->GetRasterBand(1);
    if( !gdalBand ){
        cerr << "Error fetching raster band\n";
        return NULL;
    }

    x = gdalBand->GetXSize();
    y = gdalBand->GetYSize();

    myData = new float*[x];
    for(i=0;i<x;i++) myData[i] = new float[y];

    pafScanline = new float[xsize*ysize];

    for(i=0;i<x;i++)
        for(j=0;j<y;j++)
            myData[i][j] = emptyValue;

    gdalBand->RasterIO(GF_Read,0,0,x,y,
            pafScanline,x,y,GDT_Float32,0,0);

    for(j=0;j<y;++j)
        for(int k=0;k<x;++k)
            myData[k][j] = pafScanline[j*x+k];

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

int Compact1By1(int x)
{
  x &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  x = (x ^ (x >>  1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x >>  2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x >>  4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x >>  8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
  return x;
}

int DecodeMorton2X(int code)
{
  return Compact1By1(code >> 0);
}

int DecodeMorton2Y(int code)
{
  return Compact1By1(code >> 1);
}

int Part1By1(int x)
{
  x &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  x = (x ^ (x <<  8)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x <<  4)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x <<  2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x <<  1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return x;
}

int EncodeMorton2(int x, int y)
{
  return (Part1By1(y) << 1) + Part1By1(x);
}

///TODO: Fix
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
