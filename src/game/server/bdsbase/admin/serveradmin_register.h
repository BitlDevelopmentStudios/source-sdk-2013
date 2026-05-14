#ifndef SERVERADMIN_COMMAND_REGISTER_H
#define SERVERADMIN_COMMAND_REGISTER_H

// ADD NEW MODULES HERE
#include "command\serveradmin_command_base.h"
#ifdef TF_DLL
#include "tf\admin\command\serveradmin_command_tf.h"
#endif // TF_DLL

#ifdef BDSBASE
//THEN ADD THEM HERE SO THAT THEY CAN BE LOADED
static void RegisterCommands(void)
{
	LoadBaseCommandModule(); 
#ifdef TF_DLL
	LoadTFCommandModule();
#endif // TF_DLL
}
#endif

#endif // SERVERADMIN_COMMAND_REGISTER_H