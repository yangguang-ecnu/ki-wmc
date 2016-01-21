# Introduction #

**ki-wmc** is a cross-platform framework for segmentation of the tissues and/or lesions using multispectral MRI, which is optimized to _**segmentation of white matter changes (WMC)**_

# Requirement #

The following packages and components are needed in order to compile the codes.
  * [CMake](http://www.cmake.org/) 2.6+
  * [InsightToolkit](http://www.itk.org) 4.x+
  * [TCLAP](http://tclap.sourceforge.net/)

# Getting the code #

You should clone from the repository. Currently the framework is not public and the source code is not available. You should email the project owner to obtain the code.

# Compilation and Installation #

In order to configure the codes you have to run the `init_project` script lied in `ki-wmc` folder. For UNIX systems the script is `init_project.sh` and for Windows is `init_project.bat`. This script will create a `build` folder along with `ki-wmc`. You should then have to go to the `build` directory and run the compile and install commands.

In UNIX systems, the commands should be:

```
$ cd ki-wmc
$ ./init_project.sh
$ cd ../build
$ make all
$ make install
```

# Commands #
  * wmlCollectFV
  * wmlScaleImage
  * wmlTrain
  * wmlTest
  * wmlSegmentation
  * wmlDistribution