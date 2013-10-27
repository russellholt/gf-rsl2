#include "R_RemoteConnection.h"

extern "C" res_class *Create_RemoteConnection_RC()
{
	return &(R_RemoteConnection::rslType);
}
