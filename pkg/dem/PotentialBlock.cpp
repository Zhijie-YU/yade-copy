/*CWBoon 2015 */
#ifdef YADE_POTENTIAL_BLOCKS
 //! To implement potential particles (Houlsby 2009) using sphere
#include "PotentialBlock.hpp"

YADE_PLUGIN((PotentialBlock));


PotentialBlock::~PotentialBlock()
{
}


 void PotentialBlock::addPlaneStruct() {
		   planeStruct.push_back(Planes());
}

 void PotentialBlock::addVertexStruct() {
		   vertexStruct.push_back(Vertices());
}

 void PotentialBlock::addEdgeStruct() {
		   edgeStruct.push_back(Edges());
}


void PotentialBlock::postLoad(PotentialBlock& )
{
	int planeNo = a.size();

	/* Normalize the normal vectors of the faces */
	for (int i=0; i<planeNo; i++){
		Vector3r planeNormVec = Vector3r(a[i],b[i],c[i]); 
		a[i] /= planeNormVec.norm();
		b[i] /= planeNormVec.norm();
		c[i] /= planeNormVec.norm();
		d[i] /= planeNormVec.norm();
	}

	/* Make sure the d[i] values given by the user are positive */
	for (int i=0; i<planeNo; i++){	if (d[i]<0) { a[i] *= -1; b[i] *= -1; c[i] *= -1; d[i] *= -1; } }

	for (int i=0; i<planeNo; i++){
		addPlaneStruct();
	}


     	/* CALCULATE VERTICES */
	double D[3]; double Ax[9]; 
	Eigen::Matrix3d Aplanes; 
	double dx; double dy; double dz; double Distance;
	Eigen::Matrix3d AplanesX; Eigen::Matrix3d AplanesY; Eigen::Matrix3d AplanesZ;
	Real vertCount=0; Real minDistance;

	vertices.clear();

	for (int i=0; i<planeNo; i++){
		for (int j=0; j<planeNo; j++){
			for(int k=0; k<planeNo; k++){

				Vector3r plane1 = Vector3r(a[i],b[i],c[i]);	
				Vector3r plane2 = Vector3r(a[j],b[j],c[j]);	
				Vector3r plane3 = Vector3r(a[k],b[k],c[k]);	

				double d1 = d[i]+r;
				double d2 = d[j]+r;
				double d3 = d[k]+r;

				D[0]=d1;
				D[1]=d2;
				D[2]=d3;
				Ax[0]=plane1.x(); Ax[3]=plane1.y(); Ax[6]=plane1.z();  Aplanes(0,0) = Ax[0]; Aplanes(0,1) = Ax[3]; Aplanes(0,2) = Ax[6];
				Ax[1]=plane2.x(); Ax[4]=plane2.y(); Ax[7]=plane2.z();  Aplanes(1,0) = Ax[1]; Aplanes(1,1) = Ax[4]; Aplanes(1,2) = Ax[7];
				Ax[2]=plane3.x(); Ax[5]=plane3.y(); Ax[8]=plane3.z();  Aplanes(2,0) = Ax[2]; Aplanes(2,1) = Ax[5]; Aplanes(2,2) = Ax[8];

				double detAplanes = Aplanes.determinant();

				if(fabs(detAplanes)>pow(10,-15) ){
				//if (parallel == false){

					int ipiv[3];  int bColNo=1; int info=0; /* LU */ int three =3;
					dgesv_( &three, &bColNo, Ax, &three, ipiv, D, &three, &info);
					if (info!=0){
						//std::cout<<"linear algebra error"<<endl;
					}else{
						bool inside = true; Vector3r vertex(D[0],D[1],D[2]);

						for (int m=0; m<planeNo; m++){
							Real plane = a[m]*vertex.x() + b[m]*vertex.y() + c[m]*vertex.z() - d[m]- r; if (plane>pow(10,-3)){inside = false;} 	
						}

						if (inside == true){


							/* CHECK FOR DUPLICATE VERTICES */

							if (vertCount==0) {
								vertices.push_back(vertex);
								vertCount=vertCount+1;
							} else {
								minDistance=1.0e15;
								for (unsigned int n=0; n<vertCount; n++){
									dx=(vertex.x()-vertices[n].x());
									dy=(vertex.y()-vertices[n].y());
									dz=(vertex.z()-vertices[n].z());

									Distance = sqrt( pow(dx,2.0) + pow(dy,2.0) + pow(dz,2.0) );
									if (Distance<minDistance) { minDistance = Distance; }
								}

								if ( minDistance > 1.0e-15 ) {
									vertices.push_back(vertex);
									vertCount=vertCount+1;
								}
							}

							addVertexStruct();
							int vertexID = vertexStruct.size()-1;
							vertexStruct[vertexID].planeID.push_back(i);	/*Note that the planeIDs are arranged from small to large! */
							vertexStruct[vertexID].planeID.push_back(j);    /* planeIDs are arranged in the same sequence as [a,b,c] and d */
							vertexStruct[vertexID].planeID.push_back(k);    /* vertices store information on planeIDs */

							/*Planes */
							planeStruct[i].vertexID.push_back(vertexID);	/* planes store information on vertexIDs */
							planeStruct[j].vertexID.push_back(vertexID);	
							planeStruct[k].vertexID.push_back(vertexID);	

						}
					}
				}
			}
		}
	}
}

#endif // YADE_POTENTIAL_BLOCKS
