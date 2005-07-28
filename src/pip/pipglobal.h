/*-------------------------------------------------------*/
/* pip_global.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : global definitions & variables		 */
/* create : 01/07/25				 	 */
/* update : 01/08/02                                     */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#ifndef	_PIP_GLOBAL_H_
#define _PIP_GLOBAL_H_


#ifdef  _PIPMAIN_C_
# define VAR
# define INI(x)		= x
#else
# define VAR		extern
# define INI(x)
#endif


/* ----------------------------------------------------- */
/* GLOBAL DEFINITION					 */
/* ----------------------------------------------------- */


  /* --------------------------------------------------- */
  /* �C���W�ٳ]�w                               	 */
  /* --------------------------------------------------- */


#define PIPNAME		"�d����"


  /* --------------------------------------------------- */
  /* �ӤH�ؿ��ɦW�]�w                                    */
  /* --------------------------------------------------- */


#define FN_PIP		"chicken"

VAR char *fn_pip	INI(FN_PIP);


/* ----------------------------------------------------- */
/* GLOBAL VARIABLE					 */
/* ----------------------------------------------------- */


VAR struct CHICKEN d;		/* �p������� */

VAR time_t start_time;		/* �����C���}�l�ɶ� */
VAR time_t last_time;		/* �W����s�ɶ� */

#undef VAR

#endif				/* _PIP_GLOBAL_H_ */
