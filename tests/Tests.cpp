#include <gtest/gtest.h>
#include <QtCore/QCoreApplication>

int main(int argc, char** argv)
{
    QCoreApplication application(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
