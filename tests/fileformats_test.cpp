/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "song.h"
#include "engines/gstengine.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils.h"

#include <QTemporaryFile>
#include <QResource>
#include <QDir>
#include <QSet>

namespace {

class FileformatsTest : public ::testing::TestWithParam<const char*> {
 public:
  static void SetUpTestCase() {
    sGstEngine = new GstEngine;
    ASSERT_TRUE(sGstEngine->Init());
  }

  static void TearDownTestCase() {
    delete sGstEngine;
    sGstEngine = NULL;
  }

 protected:
  FileformatsTest() {
    kFormatsWithoutMetadata.insert("wav");
  }

  void SetUp() {
    format_ = GetParam();
    resource_filename_ = ":/testdata/beep." + format_;
    temp_filetemplate_ = QDir::tempPath() + "/fileformatstest-XXXXXX." + format_;
  }

  void SaveToTempFile(QTemporaryFile* file) {
    QResource resource(resource_filename_);
    file->open();
    file->write(reinterpret_cast<const char*>(resource.data()), resource.size());
    file->flush();
  }

  static GstEngine* sGstEngine;

  QSet<QString> kFormatsWithoutMetadata;
  QString format_;
  QString resource_filename_;
  QString temp_filetemplate_;
};

GstEngine* FileformatsTest::sGstEngine = NULL;


TEST_P(FileformatsTest, Exists) {
  EXPECT_TRUE(QFile::exists(resource_filename_));
}

TEST_P(FileformatsTest, LoadsTags) {
  QTemporaryFile temp(temp_filetemplate_);
  SaveToTempFile(&temp);

  Song song;
  song.InitFromFile(temp.fileName(), -1);
  ASSERT_TRUE(song.is_valid());

  if (!kFormatsWithoutMetadata.contains(format_)) {
    EXPECT_EQ("Beep " + format_, song.title());
  }
}

TEST_P(FileformatsTest, GstCanDecode) {
  QTemporaryFile temp(temp_filetemplate_);
  SaveToTempFile(&temp);

  EXPECT_TRUE(sGstEngine->CanDecode(QUrl::fromLocalFile(temp.fileName())));
}

INSTANTIATE_TEST_CASE_P(Formats, FileformatsTest, ::testing::Values(
    "flac", "mp3", "ogg", "spx", "wav"));

}  // namespace
