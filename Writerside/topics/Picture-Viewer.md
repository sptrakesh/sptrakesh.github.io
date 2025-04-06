# Picture Viewer

A simple photo/image/… viewer built using a [QLabel](https://doc.qt.io/qt-6.8/qlabel-members.html). 
Drop directories/folders you wish to view and let the application display the image
files it finds in sequence. You can drop multiple directories/folders on to the application 
window, and it will find all viewable files and play them.

Image files are loaded in background thread(s), ensuring that the player window stays 
usable and responsive at all times. When multiple directories/folders are dropped (together 
using multiple selection or in quick succession), the directories/folders are scanned in 
parallel using multiple threads. Hence, files from different directory trees will be 
loaded interleaving each other and displayed in the sequence that the various threads 
found them.

<img src="picture-viewer-main.png" alt="Main Window" thumbnail="true"/>

<img src="picture-viewer-sample.png" alt="Sample Image" thumbnail="true"/>

## Controls
The application can be used to automatically "play" the image files it finds across directory trees,
or flip through the files manually.

* **Play** - automatically flip through all images discovered in the loaded directory 
  trees in sequence (that it was found) at intervals ranging from `1` to `15` seconds.
  Use the bottom right hand slider to control the interval at which the files are displayed.
* **Manual** - The player automatically starts displaying image files it finds. 
  Pause the player to enter into manual mode. In manual mode use the "Next" (right arrow 
  key or menu), or "Previous" (left arrow key or menu) to flip through the image files that were discovered.
* **Stop Scanning** - Useful for very large directory trees. You can stop the scan for files 
  at any given time. The player will continue to display the files that have been discovered so far.
* **Display sleep** - When left in default (checked) mode, user preferences controls (Energy Saver settings)
  whether display goes to sleep or not. If unchecked, uses the `caffeinate` command to
  prevent the display from going to sleep while app is running. For Windows, use the 
  `SetThreadExecutionState` function to achieve the same effect.
* **Show File** - Display the current displayed file in the native file manager - Finder on Mac OS X,
  and Explorer on Windows.
* **Watermark** - Render watermark text over images.
* **Create PDF** - Create PDF file with current image or all the images loaded in player window.
* **Comic Book** - Create comic book archives with all the images loaded or images in 
  the parent directory of current displayed file.

<img src="picture-viewer-menu.png" alt="Menu" thumbnail="true"/>

<img src="picture-viewer-exif.png" alt="EXIF Information" thumbnail="true"/>
