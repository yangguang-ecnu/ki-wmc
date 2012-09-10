#!/bin/sh

#aaa=-i STACKED_BRCAD032A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD036A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD038A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD046A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD058A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -m BRCAD032A_WM.nii.gz -m BRCAD036A_WM.nii.gz -m BRCAD038A_WM.nii.gz -m BRCAD046A_WM.nii.gz -m BRCAD058A_WM.nii.gz 

wmlTrain --basedir /home/soheil/Data/LONDON/train/ -i STACKED_BRCAD013A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD015A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD021A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD030A_brain_flair_T12FL_T22FL_PD2FL.nii.gz  -m BRCAD013A_WM.nii.gz -m BRCAD015A_WM.nii.gz -m BRCAD021A_WM.nii.gz -m BRCAD030A_WM.nii.gz -r 2 -s 2000 -t 0

echo =============================================

time ../build/Debug/Expansion --basedir /home/soheil/Data/LONDON/train/ -i STACKED_BRCAD064A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD065A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD068A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD072A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD077A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD079A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD089A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD104A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD105A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD122A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -i STACKED_BRCAD133A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -g BRCAD064A_WM.nii.gz -g BRCAD065A_WM.nii.gz -g BRCAD068A_WM.nii.gz -g BRCAD072A_WM.nii.gz -g BRCAD077A_WM.nii.gz -g BRCAD079A_WM.nii.gz -g BRCAD089A_WM.nii.gz -g BRCAD104A_WM.nii.gz -g BRCAD105A_WM.nii.gz -g BRCAD122A_WM.nii.gz -g BRCAD133A_WM.nii.gz -m ./model.hd -r 2 -v



time ../build/Debug/Expansion -m ./model.hd -r 2 -v --basedir /home/soheil/Data/LONDON/train/ -i STACKED_BRCAD064A_brain_flair_T12FL_T22FL_PD2FL.nii.gz -g BRCAD064A_WM.nii.gz
