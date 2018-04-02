#include "basic.h"

//这个函数只是为了gdb调试
void gdb_hk()
{
}

//void test_main()
void test()
{
    
    VosMsgHeader stMsg={0};
    daemonize();

    initData();

    init_connection();
    #if 0
    PrintUserEntry(global_UserEntryList_head);

    //test 搜寻设备接口
   struct simpleDeviceEntry *test1 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test2 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test3 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test4 = malloc_simpleDeviceEntry();

    DEBUG_PRINTF("====[%s]====test1: %d ====\n",__func__,test1);
    DEBUG_PRINTF("====[%s]====test2: %d ====\n",__func__,test2);
    DEBUG_PRINTF("====[%s]====test3: %d ====\n",__func__,test3);
    DEBUG_PRINTF("====[%s]====test4: %d ====\n",__func__,test4);
    
    PrintSimpleDeviceEntry(global_searchedDeviceEntryList_head);
    strcpy(test1->name, "wjj1");
    strcpy(test1->deviceid,"12341");

    strcpy(test2->name, "wjj2");
    strcpy(test2->deviceid,"12342");
    
    strcpy(test3->name, "wjj3");
    strcpy(test3->deviceid,"12342");
    strcpy(test4->name, "wjj4");
    strcpy(test4->deviceid,"12344");

    AddToSearchedDeviceList(test1);

    AddToSearchedDeviceList(test2);
    AddToSearchedDeviceList(test3);
    AddToSearchedDeviceList(test4);


    PrintSimpleDeviceEntry(global_searchedDeviceEntryList_head);

    //RefreshSearchedDeviceList();
    //PrintSimpleDeviceEntry(global_searchedDeviceEntryList_head);

   DEBUG_PRINTF("===========debug managedDevice list====\n");
    //测试 添加管理设备接口
   struct simpleDeviceEntry *test5 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test6 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test7 = malloc_simpleDeviceEntry();
   struct simpleDeviceEntry *test8 = malloc_simpleDeviceEntry();
   
    strcpy(test5->name, "wjj5");
    strcpy(test5->deviceid,"12345");

    strcpy(test6->name, "wjj6");
    strcpy(test6->deviceid,"12346");
    
    strcpy(test7->name, "wjj7");
    strcpy(test7->deviceid,"12346");
    
    strcpy(test8->name, "wjj8");
    strcpy(test8->deviceid,"12348");

    PrintSimpleDeviceEntry(global_ManagedDeviceEntryList_head);
    AddToManagedDeviceList(test5);
    AddToManagedDeviceList(test6);
    AddToManagedDeviceList(test7);
    AddToManagedDeviceList(test8);

    printf("==================================\n");
    RemoveFromManagedDeviceList("12345");
    PrintSimpleDeviceEntry(global_ManagedDeviceEntryList_head);

    struct simpleDeviceEntry *test9 = malloc_simpleDeviceEntry();
    strcpy(test9->name, "wjj9");
    strcpy(test9->deviceid,"12348");

    UpdateToManagedDeviceList(test9);

    PrintSimpleDeviceEntry(global_ManagedDeviceEntryList_head);


    DEBUG_PRINTF("====================test user entry ===================\n");
    struct UserEntry *user1=malloc_userEntry();
    struct UserEntry *user2=malloc_userEntry();
    struct UserEntry *user3=malloc_userEntry();

    strcpy(user1->username, "djj");
    strcpy(user1->userid,"1");

    strcpy(user2->username, "djj2");
    strcpy(user2->userid,"2");

    strcpy(user3->username, "djj3");
    strcpy(user3->userid,"3");

    AddToUserEntryList(user1);
    AddToUserEntryList(user2);
    AddToUserEntryList(user3);

    PrintUserEntry(global_UserEntryList_head);
    RemoveFromUserEntryList("2");

    PrintUserEntry(global_UserEntryList_head);

    struct UserEntry *user4=malloc_userEntry();
    strcpy(user4->username, "djj4");
    strcpy(user4->userid,"3");

    UpdateToUserEntryList(user4);
    PrintUserEntry(global_UserEntryList_head);
#endif
    //test  dev api
    struct simpleDeviceEntry test1;
    struct simpleDeviceEntry test2;
    struct simpleDeviceEntry test3;
    struct simpleDeviceEntry test4;

    strcpy(test1.deviceid, "100861");
    test1.type = 1;
    strcpy(test1.name, "door1");
    test1.status = 2;
    strcpy(test1.ieee_addr,"2001");
    test1.nw_addr=1;
    strcpy(test1.version,"1.0");
    strcpy(test1.model,"door");
    strcpy(test1.seq,"111");

    strcpy(test2.deviceid, "100862");
    test2.type = 1;
    strcpy(test2.name, "door2");
    test2.status = 2;
    strcpy(test2.ieee_addr,"2002");
    test2.nw_addr=2;
    strcpy(test2.version,"1.0");
    strcpy(test2.model,"door");
    strcpy(test2.seq,"111");
     printf("===1====\n");
     PrintSimpleDeviceEntry(global_searchedDeviceEntryList_head);

     printf("===1. managed===\n");
        PrintSimpleDeviceEntry(global_ManagedDeviceEntryList_head);
        
     AddDeviceInfo2SearchedDeviceList(&test1);
     printf("===printf searcheddevicelist===\n");
     PrintSimpleDeviceEntry(global_searchedDeviceEntryList_head);

     AddDeviceInfo2ManagedDeviceList(&test1);
     AddDeviceInfo2ManagedDeviceList(&test2);
     printf("===printf manageddevicelist===\n");
     PrintSimpleDeviceEntry(global_ManagedDeviceEntryList_head);


     //test get max row id 
     
     int num = getMaxRowIdFromDatabase("managedDevice_table");
     printf("======max row id in managedDevice_table is %d === \n", num);


     //test rename device
     RenameDevName("100861","wjj door");

     AddFingerPrint("100861", "2001", "900002");
    
    program_quit();
}
