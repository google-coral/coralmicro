#include <SD.h>

void setup() {
  SD.begin();

  Serial.println("Begin SD test");
  // Print out the directory tree
  printDirectoryTree();
  Serial.println("Should be empty");

  // Create some individual directories
  assert(SD.mkdir("/dir1"));
  assert(SD.mkdir("/dir2"));
  assert(SD.mkdir("/dir1/dir11"));
  assert(SD.mkdir("/dir1/dir13"));
  assert(SD.mkdir("/dir1/dir12/dir121"));
  assert(SD.mkdir("/dir2/dir21"));

  // Check that the directories exist
  assert(SD.exists("/dir1"));
  assert(SD.exists("/dir2"));
  assert(SD.exists("/dir1/dir11"));
  assert(SD.exists("/dir1/dir12"));
  assert(SD.exists("/dir1/dir13"));
  assert(SD.exists("/dir1/dir12/dir121"));
  assert(SD.exists("/dir2/dir21"));

  // Check that a non-existent directory doesn't exist
  assert(!SD.exists("/nonexistent"));

  // Print out the directory tree
  printDirectoryTree();

  // Open a file for writing
  SDFile file1 = SD.open("/dir1/file1", FILE_WRITE);
  SDFile file2 = SD.open("/dir2/file2", FILE_WRITE);
  assert(file1);
  assert(file2);

  // Write to the files, and check size
  auto size = file1.println("This is the file content of file1");  // adds \r\n
  assert(size == 35);

  char *buf = "Content for file2";
  size = file2.write((unsigned char *)buf, strlen(buf));
  assert(size == strlen(buf));

  // Read the files and test seek
  char readbuf[255];
  file1.seek(0);
  size = file1.read(readbuf, 255);
  readbuf[size] = 0;
  Serial.print(readbuf);

  // Close the file
  file1.close();
  assert(!file1);
  file2.close();
  assert(!file2);

  // Open read-only, and
  file1 = SD.open("/dir1/file1");
  assert(file1);

  // Seek and read one character
  assert(file1.seek(6));
  char c = file1.read();
  assert(c == 's');

  // Test peek, position, available and size
  file1.seek(8);
  c = file1.peek();
  assert(c == 't');
  c = file1.peek();
  assert(c == 't');
  c = file1.position();
  assert(c == 8);
  c = file1.available();
  if (c != 27) {
    Serial.println("Error: file1.available() != 27");
  }
  c = file1.size();
  if (c != 35) {
    Serial.println("Error: file1.size() != 35");
  }
  file1.close();
  assert(!file1);

  // Open file2 for writing, position should be at the end of file
  file2 = SD.open("/dir2/file2", FILE_WRITE);
  c = file2.position();
  if (c != 17) {
    Serial.println("Error file2.position != 17");
  }

  // Append some text to file2, and check size (should be 8 more)
  file2.println("123456");
  c = file2.size();
  if (c != 25) {
    Serial.println("Error: file2.size != 25");
  }

  // Print out the content of file2 and close it
  file2.seek(0);
  c = file2.read(readbuf, 255);
  readbuf[c] = 0;
  Serial.println(readbuf);
  file2.close();
  assert(!file2);

  // Open dir1 and test reading and rewinding it
  auto dir1 = SD.open("/dir1");
  assert(dir1.isDirectory());

  auto file = dir1.openNextFile();
  assert(!strcmp(file.name(), "/dir1/dir11"));
  dir1.rewindDirectory();
  file = dir1.openNextFile();
  assert(!strcmp(file.name(), "/dir1/dir11"));
  dir1.close();

  // Print out the directory tree
  printDirectoryTree();

  // Try to remove a non empty directory
  assert(!SD.rmdir("/dir1"));
  assert(SD.exists("/dir1"));

  // Remove all files and directories
  assert(SD.remove("/dir1/file1"));
  assert(SD.remove("/dir2/file2"));
  assert(SD.rmdir("/dir1/dir12/dir121"));
  assert(SD.rmdir("/dir1/dir11"));
  assert(SD.rmdir("/dir1/dir12"));
  assert(SD.rmdir("/dir1/dir13"));
  assert(SD.rmdir("/dir2/dir21"));
  assert(SD.rmdir("/dir1"));
  assert(SD.rmdir("/dir2"));

  // Check that none of the directories exist
  assert(!SD.exists("/dir1/dir12/dir121"));
  assert(!SD.exists("/dir1/dir11"));
  assert(!SD.exists("/dir1/dir12"));
  assert(!SD.exists("/dir1/dir13"));
  assert(!SD.exists("/dir2/dir21"));
  assert(!SD.exists("/dir1"));
  assert(!SD.exists("/dir2"));

  // Print out the directory tree
  printDirectoryTree();
  Serial.println("Should be empty");

  Serial.println("End SD test");
}

void loop() { delay(100); }

void printDirectoryTree() {
  Serial.println("Current directory tree");
  SDFile dir = SD.open("/");
  printDirectory(dir, 0);
  dir.close();
}

void printDirectory(SDFile dir, int numTabs) {
  // Prints a directory and all its subdirs
  while (true) {
    SDFile entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
