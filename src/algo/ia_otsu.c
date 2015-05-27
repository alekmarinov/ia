/*********************************************************************/
/*                                                                   */
/* Copyright (C) 2007, Alexander Marinov, Nadezhda Zlateva           */
/*                                                                   */
/* Project:       ia                                                 */
/* Filename:      ia_otsu.c                                          */
/* Description:   Histogram Multi Thresholding                       */
/*                based on the Otsu binarization method              */
/*                and realized by D.Dimov <dtdim@iinf.bas.bg>        */
/*                                                                   */
/*********************************************************************/
#include <malloc.h>
#include <math.h>
#include <ia/algo/ia_otsu.h>

/* 
 * Determines a threshold in a histogram
 * 1 - success, < 0  - error
 */
ia_int32_t ia_otsu(ia_signal_p histogram, 
				   ia_uint32_t* treshold, 
				   ia_uint32_t* mean,
				   ia_uint32_t* meanblack,
				   ia_uint32_t* meanwhite,
				   ia_double_t* areablack,
				   ia_double_t* areawhite,
				   ia_double_t* maxcriterion)
{
	ia_uint32_t NN; 
	ia_double_t NN_1;
	ia_int32_t *Omega;
	ia_int32_t *Mju;
	ia_int32_t i;
	ia_double_t muT;
	ia_int32_t opti_1, omg_1, opti_omg1; ia_double_t mu_1, opti_mu1;
	ia_int32_t opti_2, omg_2, opti_omg2; ia_double_t mu_2, opti_mu2;
	ia_double_t sigma, sigma_1, sigma_2, sigmaMax = -1.0;
	ia_int32_t t, t1;

	/* sum all histo values */
	NN = 0;
	for (t=0; t < histogram->length; t++) 
		NN += histogram->get_pixel(histogram, t);

	if ( NN == 0L) 
	{
		/* input histo is empty */
		return (-1);
	}
	else
	{
		NN_1 = 1./(double)NN;
	}

	Omega = (ia_int32_t*)malloc(histogram->length * sizeof(ia_int32_t));
	if (!Omega)
	{
		/* not enough memory */
		return (-2);
	}

	Omega[0] = histogram->get_pixel(histogram, 0);

	/* probability accummulation */
	for (i = 1; i < histogram->length; i++) 
		Omega[i] = Omega[i-1] + histogram->get_pixel(histogram, i);

	Mju = (ia_int32_t*)malloc(histogram->length*sizeof(ia_int32_t));
	if (!Mju)
	{
		/* not enough memory */
		free( (void*)Omega );
		return (-2);
	}

	/* mean value accummulation */
	Mju[0] = 0;
	for (i = 1; i < histogram->length; i++) 
		Mju[i] = Mju[i-1] + i*histogram->get_pixel(histogram, i);

	/* common mean value */
	muT = Mju[histogram->length-1];

	 /* Start thresholding (t1: 0 -:- histosize-2) */
	for (t1 = 0; t1 < histogram->length-1; t1++)
	{
		omg_1 = Omega[t1];
		if (omg_1 > 0) 
		{
			/* Mju[0] = Mju[-1] = 0.0 */
			mu_1 = (ia_double_t)Mju[t1];
			sigma_1 = mu_1*mu_1/(ia_double_t)omg_1; 
		}
		else    
		{ 
			omg_1 = 0; 
			mu_1 = sigma_1 = 0.0; 
		} /* Omega[0 -:- t1] == 0.0 */

		omg_2 = Omega[histogram->length-1] - Omega[t1];
		if (omg_2 > 0) 
		{ 
			mu_2 = (ia_double_t)(Mju[histogram->length-1] - Mju[t1]);
			sigma_2 = mu_2*mu_2/(ia_double_t)omg_2; 
		}
		else
		{ 
			omg_2 = 0; 
			mu_2 = sigma_2 = 0.0; 
		} /* Omega[t1+1 -:- histosize-1] == 0.0 */

		sigma = sigma_1 + sigma_2;

		if (sigmaMax < sigma)
		{ 
			opti_1 = t1;    
			opti_mu1 = mu_1; 
			opti_omg1 = omg_1;
			opti_2 = histogram->length-1;
			opti_mu2 = mu_2; 
			opti_omg2 = omg_2;
			sigmaMax = sigma;
		}
	}
	free( (void*)Mju );
	free( (void*)Omega );

	/* return results */
	muT *= NN_1;
	*treshold = opti_1; 
	if (mean)
	{
		*mean = (int)floor(muT+0.5);
	}

	if (meanblack)
	{
		if (opti_omg1 > 0) 
			*meanblack = (int)floor(opti_mu1/(double)opti_omg1 + 0.5); 
		else 
			*meanblack = 0;
	}

	if (meanwhite)
	{
		if (opti_omg2 > 0) 
			*meanwhite = (int)floor(opti_mu2/(double)opti_omg2 + 0.5); 
		else 
			*meanwhite = 0;
	}
	
	if (areablack)
	{
		*areablack = opti_omg1*NN_1;
	}

	if (areawhite)
	{
		*areawhite = opti_omg2*NN_1;
	}

	if (maxcriterion)
	{
		sigmaMax *= NN_1; 
		sigmaMax -= muT*muT; /* to Otsu origi criterion */
		*maxcriterion = sigmaMax;
	}
	return (1);
}

/* 
 * Determines 2 thresholds in a histogram
 * 1 - success, < 0  - error
 */
ia_int32_t ia_otsu_2(ia_signal_p histogram, 
					ia_uint32_t* treshold1, 
					ia_uint32_t* treshold2,
					ia_uint32_t* mean,
					ia_uint32_t* meanblack,
					ia_uint32_t* meangrey,
					ia_uint32_t* meanwhite,
					ia_double_t* areablack,
					ia_double_t* areagrey,
					ia_double_t* areawhite,
					ia_double_t* maxcriterion)
{
	ia_uint32_t NN;
	ia_double_t NN_1;
    ia_int32_t opti_1, omg_1, opti_omg1; 
	ia_int32_t opti_2, omg_2, opti_omg2;
	ia_int32_t opti_3, omg_3, opti_omg3;
	ia_double_t mu_1, opti_mu1;
	ia_double_t mu_2, opti_mu2;
	ia_double_t mu_3, opti_mu3;
	ia_int32_t *Mju, *Omega;
	ia_double_t muT, sigma, sigma_1, sigma_2, sigma_3, sigmaMax = -1.0;
	ia_int32_t i, t, t1, t2;

	/* sum all histo values */
	NN = 0;
	for (t=0; t < histogram->length; t++) 
		NN += histogram->get_pixel(histogram, t);

	if ( NN == 0L) 
	{
		/* input histo is empty */
		return (-1);
	}
	else
	{
		NN_1 = 1./(double)NN;
	}

	Omega = (ia_int32_t*)malloc(histogram->length * sizeof(ia_int32_t));
	if (!Omega)
	{
		/* not enough memory */
		return (-2);
	}

	Omega[0] = histogram->get_pixel(histogram, 0);

	/* probability accummulation */
	for (i = 1; i < histogram->length; i++) 
		Omega[i] = Omega[i-1] + histogram->get_pixel(histogram, i);

	Mju = (ia_int32_t*)malloc(histogram->length*sizeof(ia_int32_t));
	if (!Mju)
	{
		/* not enough memory */
		free( (void*)Omega );
		return (-2);
	}

	/* mean value accummulation */
	Mju[0] = 0;
	for (i = 1; i < histogram->length; i++) 
		Mju[i] = Mju[i-1] + i*histogram->get_pixel(histogram, i);

	/* common mean value */
	muT = Mju[histogram->length-1];

	/* Start thresholding (t1: 0 -:- histosize-3) */
	for (t1 = 0; t1 < histogram->length-2; t1++)
	{
		omg_1 = Omega[t1];                                 
		if (omg_1 > 0) 
		{ 
			mu_1 = (ia_double_t)Mju[t1];         
			sigma_1 = mu_1*mu_1/(ia_double_t)omg_1; 
		}
		else    
		{ 
			omg_1 = 0; 
			mu_1 = sigma_1 = 0.0; 
		}     

		/* Start thresholding (t2: t1+1 -:- LvlN-2) */
		for (t2 = t1+1; t2 < histogram->length-1; t2++) 
		{
			omg_2 = Omega[t2] - Omega[t1];
			if (omg_2 > 0) 
			{ 
				mu_2 = (ia_double_t)(Mju[t2] - Mju[t1]);
				sigma_2 = mu_2*mu_2/(ia_double_t)omg_2; 
			}
			else    
			{ 
				omg_2 = 0; 
				mu_2 = sigma_2 = 0.0; 
			} /* Omega[t1+1 -:- t2] == 0.0 */

			omg_3 = Omega[histogram->length-1] - Omega[t2];
			if (omg_3 > 0) 
			{ 
				mu_3 = (ia_double_t)(Mju[histogram->length-1] - Mju[t2]);
				sigma_3 = mu_3*mu_3/(ia_double_t)omg_3; 
			}
			else    
			{
				omg_3 = 0; 
				mu_3 = sigma_3 = 0.0; 
			} /* Omega[t2+1 -:- histosize-1] == 0.0 */

			sigma = sigma_1 + sigma_2 + sigma_3;

			if (sigmaMax < sigma)
			{ 
				opti_1 = t1;   
				opti_mu1 = mu_1; 
				opti_omg1 = omg_1;
				opti_2 = t2;    
				opti_mu2 = mu_2; 
				opti_omg2 = omg_2;
				opti_3 = histogram->length-1; 
				opti_mu3 = mu_3; 
				opti_omg3 = omg_3;
				sigmaMax = sigma;
			}
		}
	}
	free( (void*)Mju );
	free( (void*)Omega );

	/* return results */
	muT *= NN_1;
	*treshold1 = opti_1; 
	*treshold2 = opti_2; 
	if (mean)
	{
		*mean = (int)floor(muT+0.5);
	}

	if (meanblack)
	{
		if (opti_omg1 > 0) 
			*meanblack = (int)floor(opti_mu1/(double)opti_omg1 + 0.5); 
		else 
			*meanblack = 0;
	}

	if (meangrey)
	{
		if (opti_omg2 > 0) 
			*meangrey = (int)floor(opti_mu2/(double)opti_omg2 + 0.5); 
		else 
			*meangrey = 0;
	}

	if (meanwhite)
	{
		if (opti_omg3 > 0) 
			*meanwhite = (int)floor(opti_mu3/(double)opti_omg3 + 0.5); 
		else 
			*meanwhite = 0;
	}

	if (areablack)
	{
		*areablack = opti_omg1*NN_1;
	}

	if (areawhite)
	{
		*areawhite = opti_omg2*NN_1;
	}

	if (areagrey)
	{
		*areagrey = opti_omg3*NN_1;
	}

	if (maxcriterion)
	{
		sigmaMax *= NN_1; 
		sigmaMax -= muT*muT; /* to Otsu origi criterion */
		*maxcriterion = sigmaMax;
	}
	return (1);
}
