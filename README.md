Carcasum
===========

If you experience any trouble, please contact me.

Carcasum is a Carcassonne clone I wrote for my Master Thesis. The thesis can be found here: https://github.com/TripleWhy/Carcasum/releases/download/v1.0.0/MasterThesis.pdf

Some parts, mainly graphics and configuration files, are taken from JCloisterZone (usually marked as "jcz" in the code): http://jcloisterzone.com/en/

# Building
Requirements:
* git
* gcc >= 4.8
* Qt >= 5.2.1
* boost chrono (optional, depends on boost system)

Follow these steps:

0. Make sure git is in your PATH, otherwise qmake may fail
1. Clone the git repository
2. Check the boost dependeny. On Windows, qmake will look for a compiled boost library in ../boost_1_55_0. You can change this by editing Carcasum/Carcasum.pro. You can also disable boost by setting the USE_BOOST_THREAD_TIMER flag in Carcasum/static.h to 0. (It's not neccessary unless you want to run scientific experiments. It's needed to use the thread time for thinking timeouts instead of real time.)
3. Run qmake
4. Run make

# Binary Download (Windows only)
Download and extract https://github.com/TripleWhy/Carcasum/blob/bin/dist/Carcasum-win32.zip?raw=true.
Then run carcasum_gui.exe.

# Reseach
If you want to do reseach using Carcasum, feel free to do so.
I would appreciate a note if you do so.
Please make code you write or change publicy available also.