// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date    :
// // Version :
// // Author  :
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//

// Includes
#include "miscstatus.h"
// Constants
bool MISCStatus::DoSystemCommand(char* cmd, string &output)
{
//  FILE *fp;
//  char path[1035];
//
//  //MISC_DEBUG("%s\n", cmd);
//  output.clear();
//
//  /* Open the command for reading. */
//  fp = popen(cmd, "r");
//  if (fp == NULL) {
//    printf("Failed to run command\n");
//    return false;
//  }
//
//  /* Read the output a line at a time - output it. */
//  while (fgets(path, sizeof(path)-1, fp) != NULL) {
//    //MISC_DEBUG("%s", path);
//    output = output + path;
//  }
//  MISC_DEBUG("%s\n", output.c_str());
//
//  /* close */
//  pclose(fp);
//
  return true;
}
