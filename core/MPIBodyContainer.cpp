
#ifdef YADE_MPI
#include "MPIBodyContainer.hpp"
YADE_PLUGIN((MPIBodyContainer));
CREATE_LOGGER(MPIBodyContainer);


void MPIBodyContainer::clearContainer() {
      bContainer.clear(); 
}
#endif //YADE_MPI