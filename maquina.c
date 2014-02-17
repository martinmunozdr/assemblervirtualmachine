/*
Copyright 2010 emulador_ensamblador

“This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNUGeneral Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.“

AUTOR: Martín Muñoz del Río
Gerente de Proyectos de Simuder S.A.C www.simuder.com
martin@simuder.com

Este codigo es de una máquina virtual con 21 instrucciones basado en el ejemplo del libro:
Arquitectura de Computadoras Tercera Edición del autor M. Morris Mano

Para usarla:
1) Compilarlo
gcc -o maquina maquina.c

2) Usarlo
./maquina codigo_mv

Donde codigo_mv es el codigo objeto para esta máquina virtual

Sugerencias y opiniones al correo martinm499@gmail.com

*/


#include<stdio.h>
#include<stdlib.h>

//CABECERAS DE FUNCIONES
void leer_archivo(char *nombarch);
void leer_datos_instruccion(unsigned char inst_h,unsigned char inst_l,int *codigo_inst,int *dir,int *I);
void inicializar_arreglo_ejec(void);
int obtener_de_memoria(int dir,int I,unsigned char *memoriah, unsigned char *memorial);

//Cabeceras de funciones que ejecutan instrucciones
int AND(int inst_actual,int dir,int I);
int ADD(int inst_actual,int dir,int I);
int LDA(int inst_actual,int dir,int I);
int STA(int inst_actual,int dir,int I);
int BUN(int inst_actual,int dir,int I);
int BSA(int inst_actual,int dir,int I);
int ISZ(int inst_actual,int dir,int I);
int CLA(int inst_actual,int dir,int I);
int CLE(int inst_actual,int dir,int I);
int CMA(int inst_actual,int dir,int I);
int CME(int inst_actual,int dir,int I);
int CIR(int inst_actual,int dir,int I);
int CIL(int inst_actual,int dir,int I);
int INC(int inst_actual,int dir,int I);
int SPA(int inst_actual,int dir,int I);
int SNA(int inst_actual,int dir,int I);
int SZA(int inst_actual,int dir,int I);
int SZE(int inst_actual,int dir,int I);
int HLT(int inst_actual,int dir,int I);
int INP(int inst_actual,int dir,int I);
int OUT(int inst_actual,int dir,int I);


// VARIABLES GLOBALES

//ARREGLO DE PUNTEROS A FUNCION, 21 ELEMENTOS PUES SON 21 INSTRUCIONES
int (*ejec_instruccion[21])(int,int,int);
//MEMORIA DE LA MAQUINA VIRTUAL (AQUI DEBE CARGARSE EL PROGRAMA Y SUS DATOS)
unsigned char memoria_mv[2048];

//Declaro ACh byte mas significativo de registro AC y ACl byte menos significativo
unsigned char ACh=0,ACl=0;

int E=0; //Declaro E, 1 si hay acarreo, 0 si no hay

int main(int argc,char **argv)
{
        leer_archivo(argv[1]); //LEO EL ARCHIVO

        inicializar_arreglo_ejec(); //INICIALIZO ARREGLO DE PUNTEROS A FUNCION

        int inst_actual=0; //Direccion de instruccion a ejecutarse

        //Valores a obtener de la instruccion
        int codigo_inst=0, dir = 1;
        int I=0;
        
        //EJECUTO EL CODIGO MAQUINA (MAQUINA VIRTUAL)
        for(;inst_actual>=0 && inst_actual<=1023;)//compruebo no salga del limite
        {
            //Leo la instruccion y obtengo sus datos
            leer_datos_instruccion(memoria_mv[inst_actual*2], memoria_mv[inst_actual*2 +1],&codigo_inst,&dir,&I);

            if(dir<0 || dir>1023) //Verifico que no intente acceder a direccion no permitida
            {
                printf("La direccion %d no esta permitida.\n",dir);
                break;
            }

            //Ejecuto la instruccion
            inst_actual = ejec_instruccion[codigo_inst](inst_actual,dir,I);
        }

        return 0;
}

void leer_archivo(char *nombarch)
{
	FILE *fp;
	fp = fopen (nombarch, "r" );

	if (fp == NULL)
        {
            printf("No se pudo abrir el archivo %s\n",nombarch);
	    exit(1);
        }

        //LEO EL ARCHIVO Y LO TRANSPASO A memoria_mv
        int i=0,c=0; //Declaro contador i y variable c para recibir caracter

 	for(i=0;i<2048;i++) //Recorro y copio del archivo
        {
            c=fgetc(fp);
            if(c==EOF)
                break;
            memoria_mv[i] = (unsigned char)c;
        }

	fclose ( fp );    
}

void leer_datos_instruccion(unsigned char inst_h,unsigned char inst_l,int *codigo_inst,int *dir,int *I)
{
    *I = 0; //Inicializo Indirecto a 0 (caso mas comun)

    if( (inst_h>>2)%2 == 1) //Reviso si es Indirecto
        *I = 1;

    *codigo_inst = inst_h>>3; //Obtengo el codigo de instruccion

    *dir = 256*((int)inst_h%4) + (int)inst_l; //Obtengo parte direccion de la instruccion
}

void inicializar_arreglo_ejec(void)
{
    ejec_instruccion[0] = AND;
    ejec_instruccion[1] = ADD;
    ejec_instruccion[2] = LDA;
    ejec_instruccion[3] = STA;
    ejec_instruccion[4] = BUN;
    ejec_instruccion[5] = BSA;
    ejec_instruccion[6] = ISZ;
    ejec_instruccion[7] = CLA;
    ejec_instruccion[8] = CLE;
    ejec_instruccion[9] = CMA;
    ejec_instruccion[10] = CME;
    ejec_instruccion[11] = CIR;
    ejec_instruccion[12] = CIL;
    ejec_instruccion[13] = INC;
    ejec_instruccion[14] = SPA;
    ejec_instruccion[15] = SNA;
    ejec_instruccion[16] = SZA;
    ejec_instruccion[17] = SZE;
    ejec_instruccion[18] = HLT;
    ejec_instruccion[19] = INP;
    ejec_instruccion[20] = OUT;
}

int obtener_de_memoria(int dir,int I,unsigned char *memoriah, unsigned char *memorial)
{
    unsigned char h,l;
    h = memoria_mv[dir*2];
    l = memoria_mv[dir*2 +1];

    int dir_obtenida = dir;

    if(I)
    {
        dir_obtenida = 256*((int)h%4) + (int)l;
        *memoriah = memoria_mv[dir_obtenida*2];
        *memorial = memoria_mv[dir_obtenida*2 +1];
    }
    else
    {
        *memoriah = h;
        *memorial = l;
    }

    return dir_obtenida;
}

int AND(int inst_actual,int dir,int I)
{
    ACh = ACh & memoria_mv[dir*2];
    ACl = ACl & memoria_mv[dir*2 +1];

    return inst_actual+1;
}

int ADD(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    obtener_de_memoria(dir,I,&memoriah,&memorial);

    int acarreo = 0;

    if( (int)ACl + (int)memorial > 255)
        acarreo=1;

    ACl = ACl + memorial;

    if( (int)ACh + (int)memoriah + acarreo > 255)
        E = 1;

    ACh = ACh + memoriah + (unsigned char)acarreo;

    return inst_actual+1;
}

int LDA(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    obtener_de_memoria(dir,I,&memoriah,&memorial);

    ACh = memoriah;
    ACl = memorial;

    return inst_actual+1;
}

int STA(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    int dir_obtenida = obtener_de_memoria(dir,I,&memoriah,&memorial);

    memoria_mv[dir_obtenida*2] = ACh;
    memoria_mv[dir_obtenida*2 +1] = ACl;

    return inst_actual+1;
}

int BUN(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    if(I)
    {
        obtener_de_memoria(dir,0,&memoriah,&memorial);
        return 256*((int)memoriah%4) + (int)memorial;
    }

    return dir;
}

int BSA(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    if(I)
    {
        obtener_de_memoria(dir,0,&memoriah,&memorial);
        int dir_almacenar_retorno = 256*((int)memoriah%4) + (int)memorial;

        memoria_mv[dir_almacenar_retorno*2] = (unsigned char)(((inst_actual+1)>>8)%4);
        memoria_mv[dir_almacenar_retorno*2 +1] = (unsigned char)(inst_actual+1)%255;

        return dir_almacenar_retorno + 1;
    }

    memoria_mv[dir*2] = (unsigned char)(((inst_actual+1)>>8)%4);
    memoria_mv[dir*2 +1] = (unsigned char)(inst_actual+1)%255;

    return dir + 1;
}

int ISZ(int inst_actual,int dir,int I)
{
    unsigned char memoriah,memorial;

    int dir_obtenida = obtener_de_memoria(dir,I,&memoriah,&memorial);

    int acarreo = 0;

    if( 1 + (int)memorial > 255)
        acarreo=1;

    memorial = memorial + 1;

    memoriah = memoriah + (unsigned char)acarreo;

    //En este caso no se comprueba acarreo final

    memoria_mv[dir_obtenida*2] = memoriah;
    memoria_mv[dir_obtenida*2 +1] = memorial;

    if(memoriah == 0 && memorial == 0) //ambos son cero
        return inst_actual+2;

    return inst_actual+1;
}

int CLA(int inst_actual,int dir,int I)
{
    ACh = 0;
    ACl = 0;

    return inst_actual+1;
}

int CLE(int inst_actual,int dir,int I)
{
    E = 0;

    return inst_actual+1;
}

int CMA(int inst_actual,int dir,int I)
{
    ACh = ~ACh;
    ACl = ~ACl;

    if( 1 + (int)ACl > 255)
        ACh = ACh + 1;

    ACl = ACl + 1;

    return inst_actual+1;
}

int CME(int inst_actual,int dir,int I)
{
    E = !E;

    return inst_actual+1;
}

int CIR(int inst_actual,int dir,int I)
{
    int copiaE = E;

    if(ACl%2 == 1)
        E = 1;
    else
        E = 0;

    ACl = ACl>>1;

    if(ACh%2 == 1)
        ACl += 128;

    ACh = ACh>>1;

    if(copiaE)
        ACh += 128;

    return inst_actual+1;
}

int CIL(int inst_actual,int dir,int I)
{
    int copiaE = E;

    if(ACh>>7 == 1)
        E = 1;
    else
        E = 0;

    ACh = ACh<<1;

    if(ACl>>7 == 1)
        ACh++;

    ACl = ACl<<1;

    if(copiaE)
        ACl++;

    return inst_actual+1;
}

int INC(int inst_actual,int dir,int I)
{
    if(++ACl == 0)
        ACh++;

    return inst_actual+1;
}

int SPA(int inst_actual,int dir,int I)
{
    if(ACh>>7 == 0 && (ACh>0 || ACl>0))
        return inst_actual+2;

    return inst_actual+1;
}

int SNA(int inst_actual,int dir,int I)
{
    if(ACh>>7 == 1)
        return inst_actual+2;

    return inst_actual+1;
}

int SZA(int inst_actual,int dir,int I)
{
    if(ACh == 0 && ACl==0)
        return inst_actual+2;

    return inst_actual+1;
}

int SZE(int inst_actual,int dir,int I)
{
    if(!E)
        return inst_actual+2;

    return inst_actual+1;
}

int HLT(int inst_actual,int dir,int I)
{
    return -1; //Artificio para detener la ejecucion
}

int INP(int inst_actual,int dir,int I)
{
    ACl = (unsigned char)getchar();

    return inst_actual+1;
}

int OUT(int inst_actual,int dir,int I)
{
    putchar((char)ACl);

    return inst_actual+1;
}
