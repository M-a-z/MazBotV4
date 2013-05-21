/* ********************************************************************
 *
 * @file config.c
 * @brief simple configuration storage.
 *
 *
 * -Revision History:
 *
 *	- 0.0.2 17.08.2009/Maz  Fixed uninit
 *  - 0.0.1 17.08.2009/Maz  First draft
 *
 *
 *  Lisence info: You're allowed to use / modify this - you're only required to
 *  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
 *  if you redistribute this you're required to mention the original author:
 *  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
 *  channel.
 *
 *
 *  PasteLeft 2009 Maz.
 *  *************************************************************/


#include <config.h>
#include <generic.h>
#include <semaphore.h>

int G_cfg_inited=0;
static sem_t MbotCfgLock;
SextendedBasicConfig *config_array[E_nmbr_of_configs+1];


int cfgInit(void)
{
	if(!G_cfg_inited)
		G_cfg_inited++;
	else
	{
		WPRINTC(PrintComp_IrcCfg,"Double init for cfg storage attempted!");
		return -1;
	}
	if(0!=sem_init(&MbotCfgLock,0,1))
	{
		PPRINTC(PrintComp_IrcCfg,"Semaphore init failed at cfgInit()");
		G_cfg_inited--;
		return -1;
	}
	memset(config_array,0,sizeof(config_array));
	return 0;
}

void *GetConfig(unsigned int key,size_t *valuesize)
{
	void *datacopy=NULL;
	sem_wait(&MbotCfgLock);		//XXX: Protecting code with semaphore is terrible thing to do. Protecting data would be ok.
								//TODO: Fixme
	if(!G_cfg_inited)
	{
		EPRINTC(PrintComp_IrcCfg,"GetConfig(): config not inited!");
		goto bail_out;
	}
	if(key>=E_nmbr_of_configs)
	{
		EPRINTC(PrintComp_IrcCfg,"GetConfig(), provided key out of range!");
		goto bail_out;
	}
	if(NULL==config_array[key])
	{
		WPRINTC(PrintComp_IrcCfg,"GetConfig(), no key stored for config %d",key);
		goto bail_out;
	}
	else
	{
		MAZZERT(config_array[key]->cfg.cfgid==key,"ImpossibleHappened!");
		*valuesize=config_array[key]->cfg.datasize;
		datacopy=malloc(config_array[key]->cfg.datasize);
		if(NULL==datacopy)
		{
			PPRINTC(PrintComp_IrcCfg,"Malloc Failed at GetConfig()");
			goto bail_out;
		}
		memcpy(datacopy,config_array[key]->cfg.data,config_array[key]->cfg.datasize);
	}
bail_out:
	sem_post(&MbotCfgLock);
	return datacopy;
}

EconfigRet ConfigListenerReg(unsigned int key,configListenerF func,void *opaque)
{
	EconfigRet retval=EconfigRet_Error;
	Smbot_cfgcb *cbstruct;
	sem_wait(&MbotCfgLock);		//XXX: Protecting code with semaphore is terrible thing to do. Protecting data would be ok.
								//TODO: Fixme
	if(!G_cfg_inited)
	{
		EPRINTC(PrintComp_IrcCfg,"ConfigListenerReg(): config not inited!");
		goto goaway;
	}
	if(key>=E_nmbr_of_configs)
	{
		EPRINTC(PrintComp_IrcCfg,"ConfigListenerReg(), provided key out of range!");
		goto goaway;
	}
	cbstruct=malloc(sizeof(Smbot_cfgcb));
	if(NULL==cbstruct)
	{
		PPRINTC(PrintComp_IrcCfg,"Malloc Failed at ConfigListenerReg()");
			goto goaway;
	}
	cbstruct->func=func;
	cbstruct->opaque=opaque; //Yet another bad idea. Now we store the user's ptr straight away...
	if(NULL==config_array[key])
	{
		config_array[key]=malloc(sizeof(SextendedBasicConfig));
		memset(config_array[key],0,sizeof(SextendedBasicConfig));
		config_array[key]->listeners=mbot_ll_init();
		if(NULL==config_array[key]->listeners)
		{
			PPRINTC(PrintComp_IrcCfg,"Failed to initialize callback list for config %d!",key);
			goto goaway;
		}
		if(NULL==mbot_ll_add(config_array[key]->listeners,cbstruct))
		{
			PPRINTC(PrintComp_IrcCfg,"Adding listener for config %d FAILED!",key);
			goto goaway;
		}
	}
	else
	{
		if(NULL==config_array[key]->listeners)
		{
			config_array[key]->listeners=mbot_ll_init();
			if(NULL==config_array[key]->listeners)
			{
				PPRINTC(PrintComp_IrcCfg,"Failed to initialize callback list for config %d!",key);
				goto goaway;
			}
		}
		if(NULL==mbot_ll_add(config_array[key]->listeners,cbstruct))
		{
			PPRINTC(PrintComp_IrcCfg,"Adding listener for config %d FAILED!",key);
			goto goaway;
		}
	}
	retval = EconfigRet_Ok;
goaway:
	sem_post(&MbotCfgLock);
	return retval;
}
EconfigRet SetConfig(unsigned int key,void *value,size_t valuesize)
{
	EconfigRet retval=EconfigRet_Error;
	sem_wait(&MbotCfgLock);		//XXX: Protecting code with semaphore is terrible thing to do. Protecting data would be ok.
								//TODO: Fixme
	if(!G_cfg_inited)
	{
		EPRINTC(PrintComp_IrcCfg,"GetConfig(): config not inited!");
		goto omg_notAgain;
	}
	if(key>=E_nmbr_of_configs)
	{
		EPRINTC(PrintComp_IrcCfg,"GetConfig(), provided key out of range!");
			goto omg_notAgain;
	}
	if(NULL==config_array[key])
	{
		config_array[key]=malloc(sizeof(SextendedBasicConfig));
		if(NULL==config_array[key])
		{
			PPRINTC(PrintComp_IrcCfg,"malloc failed at SetConfig()");
			goto omg_notAgain;
		}
		memset(config_array[key],0,sizeof(SextendedBasicConfig));
		config_array[key]->cfg.datasize=valuesize;
		config_array[key]->cfg.data=malloc(valuesize);
		config_array[key]->cfg.cfgid=key;
		if(NULL==config_array[key]->cfg.data)
		{
			PPRINTC(PrintComp_IrcCfg,"malloc Failed at SetConfig()");
			free(config_array[key]);
				config_array[key]=NULL;
			goto omg_notAgain;
		}
		memcpy(config_array[key]->cfg.data,value,valuesize);
	}
	else
	{
		void *tmpval; //USe tmp var, update old only if everything goes smoothly.That way we can keep old value if something goes boom.
		//If there is callbacks, execute them.
		if(NULL!=config_array[key]->listeners)
		{
			Smbot_cfgcb *tmp;
			tmp=(Smbot_cfgcb *)mbot_ll_get_first(config_array[key]->listeners);
			while(NULL!=tmp)
			{
				//Should the value be copied??
				//Should the callback be executed on own thread?
				tmp->func(key,value,valuesize,tmp->opaque);
				tmp=(Smbot_cfgcb *)mbot_ll_get_next(config_array[key]->listeners);
			}
		}

		if(config_array[key]->cfg.cfgid==key)
		{
			WPRINTC(PrintComp_IrcCfg,"Overwriting config %d",key);
		}
		else
		{
			MAZZERT(config_array[key]->cfg.cfgid==0,"ImpossibleHappened2");
			config_array[key]->cfg.cfgid=key;
		}
		tmpval=malloc(valuesize);
		memcpy(tmpval,value,valuesize);
		if(NULL==tmpval)
		{
			PPRINTC(PrintComp_IrcCfg,"Malloc Failed at SetConfig()");
			goto omg_notAgain;
		}
		if(NULL==config_array[key]->cfg.data)
			free(config_array[key]->cfg.data);
		config_array[key]->cfg.data=tmpval;
	}
	retval=EconfigRet_Ok;
omg_notAgain:
	sem_post(&MbotCfgLock);
	return retval;
}

void cfgUninit(void)
{
	int i;
	for(i=0;i<E_nmbr_of_configs;i++)
	{
		if(NULL==config_array[i])
			continue;
		if(NULL!=config_array[i]->cfg.data)
			free(config_array[i]->cfg.data);
		if(NULL!=config_array[i]->listeners)
		{
			Smbot_cfgcb *tmp1;
			mbot_linkedList *tmp2=mbot_ll_get_first(config_array[i]->listeners);
			while(NULL!=tmp2)
			{
				if(tmp2->data!=NULL)
				{
					tmp1=(Smbot_cfgcb *)tmp2->data;
					free(tmp1->opaque);
					free(tmp2->data);
				}
				tmp2=mbot_ll_get_next(tmp2);
			}
			mbot_ll_destroy(&(config_array[i]->listeners));
		}
		free(config_array[i]);
		config_array[i]=NULL;
	}
	sem_destroy(&MbotCfgLock);
}
