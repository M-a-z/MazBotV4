#include <callbackhandler.h>
#include <generic.h>

int callbacklist_init(SServerCallbackList **lpp,ServerCallbackF cb,void *userptr)
{
	SServerCallbackList *lp;
	lp=calloc(1,sizeof(SServerCallbackList));
	*lpp=lp;
	if(NULL==lp)
	{
		PPRINT("Malloc failed when Initing callbacklist! Out of mem??");
		return -1;
	}
	lp->args.userdataptr=userptr;
	lp->callback=cb;
	lp->next=NULL;
	return 0;
}
int callbacklist_add(SServerCallbackList *list,ServerCallbackF cb,void *userptr)
{
	while(NULL!=list->next)
	{
		list=list->next;
	}
	list->next=calloc(1,sizeof(SServerCallbackList));
	if(NULL==list->next)
	{
		PPRINT("Malloc failed when adding callback to list! Out of mem??");
		return -1;
	}
	list->next->callback=cb;
    list->next->args.userdataptr=userptr;
	list->next=NULL;
	return 0;
}

