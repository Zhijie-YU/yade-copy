// (c) 2018 Bruno Chareyre <bruno.chareyre@grenoble-inp.fr>

#ifdef YADE_MPI

#include "Subdomain.hpp"
#include <core/Scene.hpp>
#include <core/BodyContainer.hpp>
#include <core/State.hpp>
#include <core/InteractionContainer.hpp>
#include <core/Interaction.hpp>
#include <pkg/common/Sphere.hpp>
#include <core/MPIBodyContainer.hpp>
#include <lib/serialization/ObjectIO.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>

// #include <lib/pyutil/numpy_boost.hpp>

YADE_PLUGIN((Subdomain));
CREATE_LOGGER(Subdomain);


YADE_PLUGIN((Bo1_Subdomain_Aabb)/*(Bo1_Facet_Aabb)(Bo1_Box_Aabb)*/);

void Subdomain::setMinMax()
{
	Scene* scene(Omega::instance().getScene().get());	// get scene
// 	Vector3r min, max;
	Real inf=std::numeric_limits<Real>::infinity();
	boundsMin=Vector3r(inf,inf,inf); boundsMax=Vector3r(-inf,-inf,-inf);
	if (ids.size()==0) LOG_WARN("empty subdomain!");
	if (ids.size()>0 and Body::byId(ids[0],scene)->subdomain != scene->subdomain) LOG_WARN("setMinMax executed with deprecated data (body->subdomain != scene->subdomain)");
	FOREACH(const Body::id_t& id, ids){
		const shared_ptr<Body>& b = Body::byId(id,scene);
		if(!b or !b->bound) continue;
		boundsMax=boundsMax.cwiseMax(b->bound->max);
		boundsMin=boundsMin.cwiseMin(b->bound->min);
	}
}

// inspired by Integrator::slaves_set (Integrator.hpp)
void Subdomain::intrs_set(const boost::python::list& source){
	int len=boost::python::len(source);
	intersections.clear();
	for(int i=0; i<len; i++){
		boost::python::extract<std::vector<Body::id_t> > serialGroup(source[i]);
		if (serialGroup.check()){ intersections.push_back(serialGroup()); continue; }
		cerr<<"  ... failed"<<endl;
		PyErr_SetString(PyExc_TypeError,"intersections should be provided as a list of list of ids");
		boost::python::throw_error_already_set();
	}
}

void Subdomain::mIntrs_set(const boost::python::list& source){
	int len=boost::python::len(source);
	mirrorIntersections.clear();
	for(int i=0; i<len; i++){
		boost::python::extract<std::vector<Body::id_t> > serialGroup(source[i]);
		if (serialGroup.check()){ mirrorIntersections.push_back(serialGroup()); continue; }
		cerr<<"  ... failed"<<endl;
		PyErr_SetString(PyExc_TypeError,"intersections should be provided as a list of list of ids");
		boost::python::throw_error_already_set();
	}
}

boost::python::list Subdomain::intrs_get(){
	boost::python::list ret;
	FOREACH(vector<Body::id_t >& grp, intersections){
		ret.append(boost::python::list(grp));
	}
	return ret;
}

boost::python::list Subdomain::mIntrs_get(){
	boost::python::list ret;
	FOREACH(vector<Body::id_t >& grp, mirrorIntersections){
		ret.append(boost::python::list(grp));
	}
	return ret;
}

void Bo1_Subdomain_Aabb::go(const shared_ptr<Shape>& cm, shared_ptr<Bound>& bv, const Se3r& se3, const Body* b){
// 	LOG_WARN("Bo1_Subdomain_Aabb::go()")
	Subdomain* domain = static_cast<Subdomain*>(cm.get());
	if(!bv){ bv=shared_ptr<Bound>(new Aabb);}
	Aabb* aabb=static_cast<Aabb*>(bv.get());

	if(!scene->isPeriodic){
		aabb->min=domain->boundsMin; aabb->max=domain->boundsMax;
		return;
	} else {LOG_ERROR("to be implemented")}
	LOG_WARN("Bo1_Subdomain_Aabb::go not implemented for periodic boundaries");
}

/********************dpk********************/

std::string Subdomain::serializeMPIBodyContainer(const shared_ptr<MPIBodyContainer>& container) {
	std::string strContainer;
	boost::iostreams::back_insert_device<std::string> inserter(strContainer);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
	yade::ObjectIO::save<decltype(container), boost::archive::binary_oarchive>(s, "container", container);
	s.flush();
	return strContainer;
}


shared_ptr<MPIBodyContainer> Subdomain::deSerializeMPIBodyContainer(const char* strContainer, int sizeC) {
	 shared_ptr<MPIBodyContainer> container (shared_ptr<MPIBodyContainer>  (new MPIBodyContainer()));
	 boost::iostreams::basic_array_source<char> device(strContainer, sizeC);
	 boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
	 yade::ObjectIO::load<decltype(container),boost::archive::binary_iarchive>(s, "container", container);
	 return container;
}


string Subdomain::fillContainerGetString(shared_ptr<MPIBodyContainer>& container, const  std::vector<Body::id_t>& ids) {
	container->insertBodyList(ids);
	std::string containerString = serializeMPIBodyContainer(container);
	return containerString;
}

string Subdomain::idsToSerializedMPIBodyContainer(const std::vector<Body::id_t>& ids){
	shared_ptr<MPIBodyContainer> container(shared_ptr<MPIBodyContainer> (new MPIBodyContainer()));
	container->insertBodyList(ids);
	return serializeMPIBodyContainer(container);
}

void Subdomain::clearSubdomainIds(){
	ids.clear();
}

void Subdomain::setIDstoSubdomain(boost::python::list& idList ){//Remark: probably no need for a function to assign a python list to vector<int>, boost::python does this very well.
	unsigned int listSize = boost::python::len(idList);
	for (unsigned int i=0; i != listSize; ++i) {
	   int  b_id = boost::python::extract<int> (idList[i]);
	   ids.push_back(b_id);// So it's not reset before filling?
	}
}

void Subdomain::getRankSize() {
	  if (!ranksSet){
	    MPI_Comm_rank(selfComm(), &subdomainRank);
	    MPI_Comm_size(selfComm(), &commSize); 
	    ranksSet = true; 
	  } else {return; }
}

// driver function for merge operation // workers send bodies, master recieves, sets the bodies into bodycontainer, sets interactions in interactionContainer.

void Subdomain::mergeOp() {

        //if (subdomainRank == master) {std::cout << "In merge operation " << std::endl; }
	getRankSize(); 
	sendAllBodiesToMaster();
	recvBodyContainersFromWorkers();
	if (subdomainRank==master){
	  Scene* scene = Omega::instance().getScene().get();
	  bool ifMerge = true;
	  processContainerStrings();
	  setBodiesToBodyContainer(scene, recvdBodyContainers, ifMerge,false ); 
	  recvdBodyContainers.clear(); 
	  bodiesSet = false; // reset flag for next merge op.
	  containersRecvd = false;
	  
	}
}

//unused at the moment, can be used to send and recv bodies between subdomains. 
void Subdomain::setCommunicationContainers() {

	//here, we setup the serialized MPIBodyContainer (MPIBodyContainer to string). it is std::vector<std::pair<container(string), sendRank/recvRank> >
	// fill the send container based on the ids from the intersection(local) map
	//Send container
	if (subdomainRank == master) {return ; }
	recvRanks.clear(); sendContainer.clear();
	unsigned int zero = 0;
	for (unsigned int i=1; i != intersections.size(); ++i){
		if ((intersections[i].size()== zero) || (i == static_cast<unsigned int>(subdomainRank))){continue ; } // exclude self or if no intersections with others)
		shared_ptr<MPIBodyContainer> container(shared_ptr<MPIBodyContainer> (new MPIBodyContainer()));
		container->subdomainRank = subdomainRank;  // used to identify the origin rank at the reciever side. (maybe not needed?)
		std::string containerString = fillContainerGetString(container, intersections[i]);
		sendContainer.push_back(std::make_pair(containerString, i));
	}
    //Recv container, here we just need the ranks for now.
	for (unsigned int i=1; i != remoteCount.size(); ++i) {
		if ( ( static_cast<unsigned int>(subdomainRank) == i) || (!remoteCount[i]) ){continue  ; }
		recvRanks.push_back(i);
	}
	commContainer = true;  //flag to check if the communicationContainers are set.
}


void Subdomain::sendContainerString() {
	//send the containers.
	if (subdomainRank == master) {return ; }
	if (! commContainer ){ LOG_ERROR("communication containers are not set!"); return; }
	for (unsigned int i=0; i != sendContainer.size(); ++i) {
	  MPI_Request request;
	  sendString(sendContainer[i].first, sendContainer[i].second, TAG_STRING+sendContainer[i].second, request);
	  mpiReqs.push_back(request);
	}
}

void Subdomain::processContainerStrings() {
	//convert the recieved string buffers to MPIBodyContainer.
	recvdBodyContainers.clear();
	if (! containersRecvd) {LOG_ERROR ("containerStrings not recvd. Fail!"); return ; }
	for (unsigned int i=0; i != recvdStringSizes.size(); ++i) {
		char *cbuf = recvdCharBuff[i]; int sz = recvdStringSizes[i];
		cbuf[sz] = '\0';
		shared_ptr<MPIBodyContainer> cntr(deSerializeMPIBodyContainer(cbuf, sz));
		recvdBodyContainers.push_back(cntr);
	}
	//free the pointers
	clearRecvdCharBuff(recvdCharBuff);
}


void Subdomain::sendAllBodiesToMaster() {
  // send all bodies from this subdomain to the master. Can be used for merge.
  // (note to self: this can be improved based on the bisection decomposition.)
	if(subdomainRank == master) {return;}
	shared_ptr<MPIBodyContainer> container(shared_ptr<MPIBodyContainer> (new MPIBodyContainer()));
	std::string s = fillContainerGetString(container, ids);
	sendStringBlocking(s, master, TAG_BODY+master);
}

void Subdomain::sendBodies(const int receiver, const vector<Body::id_t >& idsToSend) {
  // FIXME:BLABLABLA
// 	cout<<"sending from "<<subdomainRank<<" to "<<receiver<<" ids [";
// 	for(auto const& i: idsToSend) cout<<i<<" ";
// 	cout<<"]"<<endl;
	shared_ptr<MPIBodyContainer> container(shared_ptr<MPIBodyContainer> (new MPIBodyContainer()));
	std::string s = idsToSerializedMPIBodyContainer(idsToSend);
	stringBuff[receiver]=s;
	MPI_Request req;
	MPI_Isend(stringBuff[receiver].data(), s.size(), MPI_CHAR, receiver, TAG_BODY, selfComm(), &req);
	sendBodyReqs.push_back(req); 
}

void Subdomain::receiveBodies(const int sender){
// 	cout<<"receiving in "<<subdomainRank<<" from "<<sender<<" ";
	int recv_size = probeIncomingBlocking(sender, TAG_BODY);
	char *cbuf = new char[recv_size+1]; 
	recvBuffBlocking(cbuf, recv_size , TAG_BODY, sender);
	cbuf[recv_size]='\0';
	shared_ptr<MPIBodyContainer> mpiBC(deSerializeMPIBodyContainer(cbuf, recv_size));
// 	cout<<mpiBC->bContainer.size()<<" bodies"<<endl;
	std::vector<shared_ptr<MPIBodyContainer> > mpiBCVect(1,mpiBC); //setBodiesToBodyContainer needs a vector of MPIBodyContainer, so create one of size 1.
	Scene* scene = Omega::instance().getScene().get();
	setBodiesToBodyContainer(scene,mpiBCVect,false, true);
	delete[] cbuf; 
}

void Subdomain::completeSendBodies(){
	processReqs(sendBodyReqs);		// calls MPI_Wait on the reqs, cleans the vect of mpi Reqs
}

/********Functions exclusive to the master*************/

void Subdomain::initMasterContainer(){
	if (subdomainRank != master) {return; }
	 recvRanks.resize(commSize-1); 
	 for(unsigned i=0; i != recvRanks.size(); ++i ){ recvRanks[i]=i+1; }
	 recvdStringSizes.resize(commSize-1); 
	 recvdCharBuff.resize(commSize-1); 
	 allocContainerMaster = true;
}

void Subdomain::recvBodyContainersFromWorkers() {

        if (subdomainRank != master ) { return; }
        else if (subdomainRank == master){
		//if (! allocContainerMaster){ initMasterContainer(); std::cout << "init Done in  MASTER " << subdomainRank << std::endl;}
		for (int sourceRank=1; sourceRank != commSize; ++sourceRank){
			int sz =  probeIncomingBlocking(sourceRank, TAG_BODY+subdomainRank);
			recvdStringSizes[sourceRank-1] = sz; 
			int sztmp = sz+1;
			char *cbuf = new char[sztmp];
			recvBuffBlocking(cbuf, sz , TAG_BODY+subdomainRank, sourceRank);
			recvdCharBuff[sourceRank-1]=cbuf; 
		}
        containersRecvd = true;
    }
}


// set all body properties from the recvd MPIBodyContainer
void Subdomain::setBodiesToBodyContainer(Scene* scene ,std::vector<shared_ptr<MPIBodyContainer> >& containers, bool ifMerge, bool resetInteractions) {
	// to be used when deserializing a recieved container.
	shared_ptr<BodyContainer>& bodyContainer = scene->bodies;
	shared_ptr<InteractionContainer>& interactionContainer = scene->interactions; 
	for (unsigned int i=0; i != containers.size(); ++i){
		shared_ptr<MPIBodyContainer>& mpiContainer = containers[i];
		std::vector<shared_ptr<Body> >& bContainer = mpiContainer->bContainer;
		for (auto bIter = bContainer.begin(); bIter != bContainer.end(); ++ bIter) {
			shared_ptr<Body> newBody = *(bIter);
			// check if the body already exists in the existing bodycontainer
			const Body::id_t& idx = newBody->id;
			std::map<Body::id_t, shared_ptr<Interaction> > intrsToSet = newBody->intrs;
			shared_ptr<Body>& b = (*bodyContainer)[idx];
			if (ifMerge){
				/* FIXME: this switcheroo of material for existing bodies is done to avoid "RuntimeError: Scene::postLoad: Internal inconsistency,*/ 
				/*shared materials not preserved when loaded; please report bug." during global merge, splits.*/
				shared_ptr<Material> tmpMat = b->material; 
				b = newBody; 
				b->material = tmpMat; 
				b->intrs.clear(); 
			}else{
				if (!b) newBody->intrs.clear(); //we can clear here, interactions are stored in intrsToSet
				else newBody->intrs=b->intrs;
				b = newBody; 
				//bodyContainer->insertAtId(newBody, newBody->id);  
			}
	//if(!resetInteractions)
			for (auto mapIter = intrsToSet.begin(); mapIter != intrsToSet.end(); ++mapIter){
				const Body::id_t& id1 = mapIter->second->id1; const Body::id_t& id2 = mapIter->second->id2;
				if ((*bodyContainer)[id1] and (*bodyContainer)[id2] ) {// we will insert interactions only when both bodies are inserted
					/* FIXME: we should make really sure that we are not overwriting a live interaction with a deprecated one (possible solution: make all interactions between remote bodies virtual)*/
					scene->interactions->insertInteractionMPI(mapIter->second);
				}
			}
		}
	}
	interactionContainer->dirty = true;  //notify the collider about the new interactions/new body. 
	containers.clear();
	bodiesSet = true;
}


void Subdomain::splitBodiesToWorkers(const bool& eraseWorkerIds){

	if (!eraseWorkerIds){return; } 
	shared_ptr<Scene> scene = Omega::instance().getScene(); 
	shared_ptr<BodyContainer> bodyContainer = scene-> bodies; 
	std::vector<std::vector<Body::id_t> > idsToSend; 
	
	if (subdomainRank==0){
		idsToSend.resize(commSize-1); 
		for (const auto& b : bodyContainer->body){
			if (!b->getIsSubdomain()){
				if (b->subdomain != master){idsToSend[b->subdomain-1].push_back(b->id);}
				for (const auto& bIntrs : b->intrs){
					 Body::id_t otherId; 
					shared_ptr<Interaction> I = bIntrs.second; 
					if (b->id == I->getId1()){otherId = I->getId2(); }else{otherId = I->getId1();}
					const shared_ptr<Body>& otherBody = (*bodyContainer)[otherId]; 
					if (otherBody->getIsSubdomain()){
						idsToSend[otherBody->subdomain-1].push_back(b->id); 
					}
				}
			}
		}
	}
	
	if ((subdomainRank != master) && (eraseWorkerIds)){
		for(const auto& b : bodyContainer->body){
			if (!b){continue; }
			if (b->subdomain ==0){continue; }
			if (!b->getIsSubdomain()){bodyContainer->erase(b->id, true); }
		}
		clearSubdomainIds(); 
	}
	
	if (subdomainRank == master) {
		for (unsigned rnk = 0; rnk != idsToSend.size(); ++rnk){
			const std::vector<Body::id_t>& workerIds = idsToSend[rnk]; 
			shared_ptr<MPIBodyContainer> container(shared_ptr<MPIBodyContainer> (new MPIBodyContainer()));
			std::string s = fillContainerGetString(container,workerIds); 
			sendStringBlocking(s, rnk+1, TAG_BODY); 
			
		}
		
	}
	
	
	if (subdomainRank != master){
		receiveBodies(master); 
		for (const auto& b : bodyContainer->body){
			if (!b){continue; }
			if (!b->getIsSubdomain()){
				if (b->subdomain==subdomainRank){
				ids.push_back(b->id); }
			}
		}
		
	}

}


/*********************communication functions**************/
//blocking  send and recv

void Subdomain::sendStringBlocking(std::string& s, int destRank, int tag) {

	MPI_Send(s.data(), s.size(), MPI_CHAR, destRank, tag, selfComm());
}

int Subdomain::probeIncomingBlocking(int sourceRank, int tag) {
	MPI_Status status;
	MPI_Probe(sourceRank, tag, selfComm(), &status);
	int sz;
	MPI_Get_count(&status, MPI_CHAR, &sz);
	return sz;
}

void Subdomain::recvBuffBlocking(char* cbuf, int cbufSz, int tag, int sourceRank){
	MPI_Status status;
	MPI_Recv(cbuf, cbufSz, MPI_CHAR, sourceRank, tag, selfComm(), &status);
}



//non-blocking calls  --> Isend, Iprobe, Irecv, Waitall();  (we will use mpi_isend + mpi_wait and then mpi_probe followed by mpi_recv)

void Subdomain::sendString(std::string& s, int destRank, int tag,  MPI_Request& request){

	int len = s.size();
	MPI_Isend(s.data(), len, MPI_CHAR,  destRank, tag , selfComm(), &request);  //

}

int Subdomain::probeIncoming(int sourceRank, int tag) {
	int flag=0; MPI_Status status;
	while (! flag) {
	  MPI_Iprobe(sourceRank, tag, selfComm(), &flag, &status);
	}
	int sz;
	MPI_Get_count(&status, MPI_CHAR, &sz);
	return sz; mpiStatus.push_back(status);

}

void Subdomain::recvBuff(char* cbuf, int cbufsZ, int sourceRank, MPI_Request& request) {

	MPI_Irecv(cbuf, cbufsZ, MPI_CHAR, sourceRank, TAG_STRING + subdomainRank, selfComm(), &request);
}

void Subdomain::processReqs(std::vector<MPI_Request>& mpiReqs) {
	if (!mpiReqs.size()){return ; }
	for (unsigned int i = 0; i != mpiReqs.size(); ++i){
	  MPI_Status status;
	  MPI_Wait(&mpiReqs[i], &status);
    }

    resetReqs(mpiReqs);
}

void Subdomain::resetReqs(std::vector<MPI_Request>& mpiReqs) {
	mpiReqs.clear();
}

void Subdomain::processReqsAll(std::vector<MPI_Request>& mpiReqs, std::vector<MPI_Status>& mpiStats) {

	for (unsigned int i = 0 ; i != mpiReqs.size(); ++i){
	  //MPI_Status status;
	  MPI_Waitall(1, &mpiReqs[i], &mpiStats[i]);
        }
        mpiStats.clear();
        resetReqs(mpiReqs);
}


void Subdomain::clearRecvdCharBuff(std::vector<char*>& rcharBuff) {

	for (std::vector<char*>::iterator cIter = rcharBuff.begin(); cIter != rcharBuff.end(); ++cIter){
	    delete[] (*cIter);
	  }
	if (subdomainRank != master){ rcharBuff.clear(); } // assuming master alwasy recieves from workers, hence the size of this vector for master is fixed.
}

#endif
