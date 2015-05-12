PALAIS
http://www.palais.io/

A 3D Sandbox Environment for Artificial Intelligence in Games.

Currently available for Windows (from XP) and Mac OS X (from 10.6). Binaries can be downloaded from the website.

Tutorials, documentation and further info can be found at:
http://www.palais.io/

License:
The project itself is MIT-Licensed, but we include libraries, fonts and art under different open source licenses, see sandbox/licenses/.

Usage:
See the samples/ directory and the website for sample code and documentation.

Build Instructions:
- Get Ogre 1.8
- Get Qt 5+
- Get bullet 2.82

Win32 Build Instructions:
- set environment variables (Advanced System Settings -> Advanced -> Environment Variables):
 OGRE_HOME=${PATH_TO_OgreSDK}, and
 BULLET_HOME=${PATH_TO_BulletSDK}

OS X 10.10 Build Instructions:
- launchctl setenv OGRE_HOME ${PATH_TO_OgreSDK}
- launchctl setenv BULLET_HOME ${PATH_TO_BulletSDK}

Uncomment the win/macdeployqt lines in the sandbox/sandbox.pro file to build deployment packages for Windows / OS X.