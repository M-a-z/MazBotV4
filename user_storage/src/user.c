#include <user.h>


SIRCuserHandler *StorageInit(EuserHandlerType type, EuserIdentMode mode)
{
	switch(type)
	{
		case EuserHandlerType_Permanent:
			return (SIRCuserHandler *)PermUserStorageInit(type,mode);
			break;
		case EuserHandlerType_Online:
			return TempUserStorageInit(type,mode);
		default:
			EPRINT("Invalid storage type given to UserStorageInit!");
			break;
	}
	return NULL;
}

