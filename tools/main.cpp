//
// Created by rausa on 20/12/22.
//


#include <stdio.h>
/* cgnslib.h file must be located in directory specified by -I during compile: */
#include "cgnslib.h"

#if CGNS_VERSION < 3100
# define cgsize_t int
#endif


int main (int args, char **input){

    float x[21*17*9],y[21*17*9],z[21*17*9];
    cgsize_t isize[3][1],ielem[20*16*8][8];
    int index_file,index_base,index_zone;
    cgsize_t irmin,irmax,istart,iend;
    int nsections,index_sect,nbndry,iparent_flag;
    cgsize_t iparentdata;
    char zonename[33],sectionname[33];
    CGNS_ENUMT(ElementType_t) itype;

/* READ X, Y, Z GRID POINTS FROM CGNS FILE */
/* open CGNS file for read-only */
    if (cg_open("Meshes/prova.cgns",CG_MODE_READ,&index_file)) cg_error_exit();


  return 0;

}