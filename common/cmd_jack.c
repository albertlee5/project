#include <common.h>
#include <config.h>
#include <command.h>


int do_jack(){
  printf("just for test!\n");
}

U_BOOT_CMD(jack,0,1,do_jack,
        "Test program.\n", "Duh?\ns");

