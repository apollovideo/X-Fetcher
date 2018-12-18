# X-Fetcher
X-Fetcher project for Berlin demo.

## Build/Run Notes
The QT version is 5.10.1

Default parameters are included in `demosettings.ini`. You may need to change them for your environment.
Build instructions:

For Linux, clone XmlRpc-Linux-X-Fetcher into .. because the Qt project uses a relative path. From that directory, run make to create libxmlrpc.a. Install ffmpeg (sudo apt-get install ffmpeg).

Use qtcreator to build the program `X-Fetcher-Linux`.

Run `X-Fetcher-Linux` without any command-line parameters. It expects to find `demosettings.ini` in its current directory.
