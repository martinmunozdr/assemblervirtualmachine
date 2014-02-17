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
gcc -o ensamblador ensamblador.c

2) Usarlo
./ensamblador codigo codigo_mv

Donde codigo es el fuente y codigo_mv es el objeto

3) Ejecutar el objeto generado por el ensamblador
./maquina codigo_mv


Sugerencias y opiniones al correo martinm499@gmail.com

*/


#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <ctype.h>

//CABECERAS DE FUNCIONES
void leer_archivo(char *nombarch);
void buscar_etiqueta(char *linea,int num_linea);
void armar_tabla_etiquetas(void);
void nueva_etiqueta(char *linea,int pos_dos_puntos,int num_linea);
void limpiar_memoria(void);
void ensamblar(void);
void crear_codigo_mv(char *linea,int num_linea);
void validar(char *cadena);
void caracteres_a_numero(char *cadena);
void obtener_txt(char *inst_o_dato,char *texto);
int pseudo_instruccion(char *inst_o_dato,int num_linea);
void filtrar_etiqueta(char *linea,char *inst_o_dato);
void instruccion_o_dato(char *inst_o_dato,int num_linea);
void agregar_a_codigo_mv(int entero);
void obtener_elemento(char *inst_o_dato,char * ps_inst,int elemento,int longitud);
int obtener_direc(char *inst_o_dato);
int indice_eti(char *eti);
void generar_archivo_mv();
int buscar_pseudo_org(char *linea, int num_linea);

//VARIABLES GLOBALES
char nombre_exe_mv[50];

char fuente[15000];

char memoria_mv[2048];
short int pos_memoria = 0;

short int num_etiquetas=0;
char *etiquetas[1024];
int direccion_etiqueta[1024];

int main(int argc, char **argv)
{
        if(argc<3)
        {
            printf("\nIngrese los dos parametros \n");
            return 0;
        }

        leer_archivo(argv[1]); //LEO EL ARCHIVO

        strcpy( nombre_exe_mv,argv[2]); //Almaceno el nombre del archivo a generar

        armar_tabla_etiquetas();
        ensamblar();
        generar_archivo_mv();

        return 0;
}

void leer_archivo(char *nombarch) //Leo el archivo sin comentarios
{
    FILE *fp;
    fp = fopen (nombarch, "r" );

    if (fp == NULL)
    {
        printf("No se pudo abrir el archivo %s\n",nombarch);
        exit(1);
    }

    //LEO EL ARCHIVO Y LO TRANSPASO a fuente sin comentarios ni lineas vacias
    //Declaro contadores i,k variable c para recibir caracter y banderas comentario y vacia
    int i=0,k=0,c=0,comentario=0,vacia=1;

    for(i=0;i<15000;i++) //Recorro y copio el archivo
    {
        c=fgetc(fp);
        if(c==';')
        {
            comentario = 1;
            if(!vacia)
                fuente[k++] = '\n';
        }
        if(c!=' ' && c!='\t' && c!='\n' && c!='\0' && c!=';')
            vacia=0;
        if(c==EOF)
        {
            fuente[k] = '\0';
            break;
        }
        if(!comentario && !vacia) //Copio solo si no es comentario, y no es linea vacia
            fuente[k++] = (char)c;
        if(c=='\n' || c=='\0')
        {
            comentario = 0;
             vacia = 1;
        }
    }

    fclose ( fp );    
}

void armar_tabla_etiquetas(void)
{
    int i=0,k=0,num_linea=0;
    char linea[50];
    for(;i<strlen(fuente);i++)
    {
        if(fuente[i]!='\n')
            linea[k++] = fuente[i];

        linea[k]='\0';

        if(fuente[i]=='\n' || fuente[i+1]=='\0' )
        {
            buscar_etiqueta(linea,num_linea);
            num_linea++;
            k=0;
            num_linea = buscar_pseudo_org(linea,num_linea);
        }
    }
}

int buscar_pseudo_org(char *linea, int num_linea)
{
    char pseudo[5],valor[10];

    obtener_elemento(linea,pseudo,1,5);

    if(strcmp(pseudo,"ORG")!=0)
        return num_linea;

    obtener_elemento(linea,valor,2,10);

    return atoi(valor);
}

void buscar_etiqueta(char *linea,int num_linea)
{
    int i=0;
    for(;i<strlen(linea);i++)
        if(linea[i]==':')
        {
            nueva_etiqueta(linea,i,num_linea);
            break;
        }
}

void nueva_etiqueta(char *linea,int pos_dos_puntos,int num_linea)
{
    char etiqueta[36];
    int i=0,k=0;
    for(;i<pos_dos_puntos && k<35;i++)
        if(linea[i]!=' ') //elimina espacios, inclusive los que esten en el centro de la etiqueta
            etiqueta[k++] = linea[i];

    etiqueta[k]='\0';

    if(isdigit(etiqueta[0]))
    {
        limpiar_memoria();
        printf("\nERROR: la etiqueta %s empieza con digito y no con letra\n",etiqueta);
        exit(1);
    }

    etiquetas[num_etiquetas] = (char *)malloc(sizeof(char)*(strlen(etiqueta)+1));
    strcpy(etiquetas[num_etiquetas],etiqueta);
    direccion_etiqueta[num_etiquetas++] = num_linea;
}

void limpiar_memoria(void)
{
    int i=0;
    for(;i<num_etiquetas;i++)
        free(etiquetas[i]);
}

void ensamblar(void)
{
    int i=0,k=0,num_linea=0;
    char linea[50];
    for(;i<strlen(fuente);i++)
    {
        if(fuente[i]!='\n')
            linea[k++] = fuente[i];

        linea[k]='\0';

        if(fuente[i]=='\n' || fuente[i+1]=='\0' )
        {
            crear_codigo_mv(linea,num_linea);          
            num_linea++;
            k=0;
        }
    }
}

void crear_codigo_mv(char *linea,int num_linea)
{
    char inst_o_dato[50];
    filtrar_etiqueta(linea,inst_o_dato);
    if(!pseudo_instruccion(inst_o_dato,num_linea))
        instruccion_o_dato(inst_o_dato,num_linea);
}

void instruccion_o_dato(char *inst_o_dato,int num_linea)
{
    int cod_inst,dir,I;
    cod_inst = obtener_cod_instruc(inst_o_dato);
    dir = obtener_direc(inst_o_dato);
    I = obtener_I(inst_o_dato);
    char etiqueta_apuntada[36];
    
    if(cod_inst<0)
    {
        obtener_elemento(inst_o_dato,etiqueta_apuntada,1,36);
        if(isalpha(etiqueta_apuntada[0]))
        {
            agregar_a_codigo_mv(direccion_etiqueta[indice_eti(etiqueta_apuntada)]);
            return;
        }
        agregar_a_codigo_mv(atoi(inst_o_dato));
        return;
    }
    if(cod_inst<7 && dir==-1)
    {
        printf("\nERROR: se esperaba direccion o etiqueta en %s\n",inst_o_dato);
        exit(1);
    }
    if(cod_inst>=7 && dir!=-1)
    {
        printf("\nERROR: No se esperaba direccion ni etiqueta en %s\n",inst_o_dato);
        exit(1);
    }

    if(I == -1)
    {
        printf("\nERROR: Instruccionmal mal formada %s\n",inst_o_dato);
        exit(1);
    }
    if(dir == -1)
        dir=0;

    agregar_a_codigo_mv(dir + 1024*I + 2048*cod_inst);
}

int obtener_cod_instruc(char *inst_o_dato)
{
    char inst[5];
    obtener_elemento(inst_o_dato,inst,1,5);

    if(strcmp(inst,"AND")==0)
        return 0;
    if(strcmp(inst,"ADD")==0)
        return 1;
    if(strcmp(inst,"LDA")==0)
        return 2;
    if(strcmp(inst,"STA")==0)
        return 3;
    if(strcmp(inst,"BUN")==0)
        return 4;
    if(strcmp(inst,"BSA")==0)
        return 5;
    if(strcmp(inst,"ISZ")==0)
        return 6;
    if(strcmp(inst,"CLA")==0)
        return 7;
    if(strcmp(inst,"CLE")==0)
        return 8;
    if(strcmp(inst,"CMA")==0)
        return 9;
    if(strcmp(inst,"CME")==0)
        return 10;
    if(strcmp(inst,"CIR")==0)
        return 11;
    if(strcmp(inst,"CIL")==0)
        return 12;
    if(strcmp(inst,"INC")==0)
        return 13;
    if(strcmp(inst,"SPA")==0)
        return 14;
    if(strcmp(inst,"SNA")==0)
        return 15;
    if(strcmp(inst,"SZA")==0)
        return 16;
    if(strcmp(inst,"SZE")==0)
        return 17;
    if(strcmp(inst,"HLT")==0)
        return 18;
    if(strcmp(inst,"INP")==0)
        return 19;
    if(strcmp(inst,"OUT")==0)
        return 20;
    else
        return -1;
}

int obtener_I(char *inst_o_dato)
{
    char inst[3];
    obtener_elemento(inst_o_dato,inst,3,3);

    if(strcmp(inst,"I")==0)
        return 1;
    if(strlen(inst)==0)
        return 0;
    else
        return -1;
}

int obtener_direc(char *inst_o_dato)
{
    char direc[40];
    obtener_elemento(inst_o_dato,direc,2,40);

    if(strlen(direc)==0)
        return -1;
    else if(!isdigit(direc[0]))
        return direccion_etiqueta[indice_eti(direc)];
    else
        return atoi(direc);
}

int indice_eti(char *eti)
{
    int indice = -1,i=0;

    for(;i<num_etiquetas;i++)
        if(strcmp(etiquetas[i],eti)==0)
        {
            indice = i;
            break;
        }

    if(indice==-1)
    {
        printf("La etiqueta: %s no existe\n",eti);
        exit(1);
    }

    return indice;
}

void agregar_a_codigo_mv(int entero)
{
    memoria_mv[pos_memoria*2] = (unsigned char)((entero>>8)%256);
    memoria_mv[pos_memoria++ *2 +1] = (unsigned char)(entero%256);
}

void generar_archivo_mv()
{
    FILE *fp;

    fp = fopen ( nombre_exe_mv, "w" );

    if (fp == NULL)
    {
        printf("\nNo se pudo abrir el archivo para generar el codigo maquina virtual\n");
        exit(1);
    }

    int i=0;
    for(;i<2048;i++)
        fputc(memoria_mv[i],fp);

    fclose ( fp );
}

void filtrar_etiqueta(char *linea,char *inst_o_dato)
{
    int i=0;
    for(;i<strlen(linea);i++)
        if(linea[i]==':')
            break;

    if(i==strlen(linea)) //no encontró ':'
        strcpy(inst_o_dato,linea);
    else
        strcpy(inst_o_dato,&linea[i+1]);
}

int pseudo_instruccion(char *inst_o_dato,int num_linea)
{
    char ps_inst[5],texto[7],valor[10];
    obtener_elemento(inst_o_dato,ps_inst,1,5);

    if(strcmp(ps_inst,"TXT")==0)
    {
        obtener_txt(inst_o_dato,texto);
        validar(texto);
        caracteres_a_numero(texto);

        return 1;
    }
    else if(strcmp(ps_inst,"ORG")==0)
    {
        obtener_elemento(inst_o_dato,valor,2,10);
        pos_memoria = atoi(valor);

        return 1;        
    }
    else
        return 0;
}

void obtener_elemento(char *inst_o_dato,char * ps_inst,int elemento,int longitud)
{
    int i=0,k=0,num_elem=1;

    for(;i<strlen(inst_o_dato);i++)
    {
        if(inst_o_dato[i]==' ' && i>0 && inst_o_dato[i-1]!=' ')
            num_elem++;
        if(inst_o_dato[i]!=' ' && num_elem == elemento)
            break;
    }

    for(;i<strlen(inst_o_dato);i++)
        if(inst_o_dato[i]!=' ')
        {
            if(k<longitud-1)
                ps_inst[k++] = inst_o_dato[i];
        }
        else
            break;

    ps_inst[k] = '\0';
}

void obtener_txt(char *inst_o_dato,char *texto)
{
    int i=0, k=0;
    for(;i<strlen(inst_o_dato);i++)
        if(inst_o_dato[i]=='T' && inst_o_dato[i+1]=='X' && inst_o_dato[i+2]=='T')
        {
            i+=3;
            break;
        }

    for(;i<strlen(inst_o_dato);i++)
        if(inst_o_dato[i]=='\'')
            break;
        else
        {
            if(inst_o_dato[i]!=' ' && inst_o_dato[i]!='\t')
            {
                printf("ERROR: Se esperaba comilla en: %s\n",inst_o_dato);
                exit(1);
            }
        }

    for(;i<strlen(inst_o_dato);i++)
        if(k<6)
        {
            texto[k++] = inst_o_dato[i];
            texto[k] = '\0';
            if(inst_o_dato[i]=='\'' && inst_o_dato[i-1]!='\\' && k!=1)
                break;
        }
        else
        {
            printf("\nERROR: Demasiado texto para la pseudo instruccion TXT %s\n",texto);
            exit(1);
        }
}

void validar(char *cadena)
{
    int cantidad_comillas=0, i=0;

    for(;i<strlen(cadena);i++)
        if((cadena[i]=='\'' && i==0) || (cadena[i]=='\'' && cadena[i-1]!='\\'))
           cantidad_comillas++;

    if(cantidad_comillas!=2)
    {
        printf("ERROR en cantidad de comillas encontradas en: %s\n",cadena);
        exit(1);
    }

    for(i=1;i<strlen(cadena)-1;i++)
        if(cadena[i]=='\\')
        {
           if(cadena[i+1]!='\\' && cadena[i+1]!='\'' && cadena[i+1]!='n' && cadena[i+1]!='t')
           {
               printf("\nERROR: Solo se aceptan \\n \\' \\\\ \\t \n");
               exit(1);
               break;
           }
           else
               i++;
        }
}

void caracteres_a_numero(char *cadena)
{
    int i=1,resultado=0;
    for(;i<strlen(cadena)-1;i++)
    {
        if(cadena[i]=='\\')
        {
            switch(cadena[++i])
            {
                case 'n':
                    resultado = resultado*256 + '\n';
                    break;
                case 't':
                    resultado = resultado*256 + '\t';
                    break;
                case '\\':
                    resultado = resultado*256 + '\\';
                    break;
            }
        }
        else
            resultado = resultado*256 + cadena[i];
    }

    agregar_a_codigo_mv(resultado);
}
