//Copyright (C) 2026 kwmsnode
//
//  This program is free software:
//  you can redistribute it and/or modify
//  it under the terms of the GNU General Public License.
// EX Extended Font Kit
// EX拡張フォントキット
// 非公式開発環境でEXFフォントを扱う。
// アルファ0.1バージョン。GPL、保証なし。
// kwmsnodeが作成。

#include <graphics/drawing.h>
#include <graphics/color.h>
#include <graphics/text.h>
#include <graphics/lcdc.h>
#include <sh4a/input/keypad.h>
#include <syscalls/syscalls.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "exefkit.h"

// ファイル探索時のフルパス格納用に保持する変数のバイト数。実際のフルパスがこれを超えるとリターン。（snprintfは現時点で使用不能。）
// 参考までに、FAT32は最大260バイト（Windows）。インターナルストレージのファイルシステムは最大12バイト。C言語での終端文字は1バイト。
#define MAXPATHLEN 300

// グローバルファイルディスクリプタを未使用に（初期化で使用）
int globalfd = -1;

//EXFIDから初期化
int exefkit_init_exfid(const unsigned char* exfid){
  if(exfid == NULL) return -1;
  // グローバルファイルディスクリプタが0以上（使用中）ならそれを閉じて未使用に。
  if(globalfd >= 0){
      sys_close(globalfd);
      globalfd = -1;
  }
  // 見つかったファイル名一覧を保存する配列を作成。
  char namelistdrv[8][256];
  char namelistcrd[8][256];
  // 作った配列に見つかったファイル名一覧を格納、返り値を変数に保存。
  const int exfcountdrv = exefkit_getexflist("\\\\drv0\\", namelistdrv);
  const int exfcountcrd = exefkit_getexflist("\\\\crd0\\exfs\\", namelistcrd);
  // マジックナンバー。
  const unsigned char* magic = "EXEXFONT";
  // 検索に使用するファイルディスクリプタ、結果を保持する変数。
  int fd = -1;
  int result = -1;
  // 読み込んだマジックナンバーとEXFIDを保持する変数。
  unsigned char readmagic[8];
  unsigned char readexfid[16];
  // 作成したフルパスを保存する変数。
  char fullpath[MAXPATHLEN];
  // drv0を検索する。
  for(int i = 0; i < exfcountdrv; i++){
    // フルパスを生成。
    if(strlen(&namelistdrv[i][0]) + strlen("\\\\drv0\\") + 1 >= MAXPATHLEN) continue;
    sprintf(&fullpath[0], "%s%s", "\\\\drv0\\", &namelistdrv[i][0]);
    // 生成したフルパスからファイルを開く。FDが0未満（エラー）ならそのファイルをスキップ。
    fd = sys_open(&fullpath[0], FILE_RD);
    if(fd < 0) continue;
    // 先頭8バイトを読み込み変数に保存。読み込んだのが8バイトでないならファイルを閉じてスキップ。
    result = sys_read(fd, &readmagic[0], 8);
    if(result != 8){ sys_close(fd); continue; }
    // マジックナンバーと読み込んだ8バイトが違うならファイルを閉じてスキップ。
    if(memcmp(&magic[0], &readmagic[0], 8) != 0){ sys_close(fd); continue; }
    // 読み込み位置を16バイト目にずらす。結果が0未満（エラー）ならファイルを閉じてスキップ。
    result = sys_seek(fd, 16, SEEK_SET);
    if(result < 0){ sys_close(fd); continue; }
    // 16バイト目から16バイト（EXFID）を読み込み変数に保存。読み込んだのが16バイトでないならファイルを閉じてスキップ。
    result = sys_read(fd, &readexfid[0], 16);
    if(result != 16){ sys_close(fd); continue; }
    // 読み込んだものと渡されたEXFIDを比較。0（オーケー）なら処理中のファイルディスクリプタをグローバルFDに保存して関数を終了。
    if(memcmp(&readexfid[0], &exfid[0], 16) == 0){
      globalfd = fd;
      return 0;
    }
    // 見つからないならファイルを閉じる。次の処理へ。
    sys_close(fd);
  }
  // crd0を検索する。
  for(int i = 0; i < exfcountcrd; i++){
    // フルパスを生成。
    if(strlen(&namelistcrd[i][0]) + strlen("\\\\crd0\\exfs\\") + 1 >= MAXPATHLEN) continue;
    sprintf(&fullpath[0], "%s%s", "\\\\crd0\\exfs\\", &namelistcrd[i][0]);
    // 生成したフルパスからファイルを開く。FDが0未満（エラー）ならそのファイルをスキップ。
    fd = sys_open(&fullpath[0], FILE_RD);
    if(fd < 0) continue;
    // 先頭8バイトを読み込み変数に保存。読み込んだのが8バイトでないならファイルを閉じてスキップ。
    result = sys_read(fd, &readmagic[0], 8);
    if(result != 8){ sys_close(fd); continue; }
    // マジックナンバーと読み込んだ8バイトが違うならファイルを閉じてスキップ。
    if(memcmp(&magic[0], &readmagic[0], 8) != 0){ sys_close(fd); continue; }
    // 読み込み位置を16バイト目にずらす。結果が0未満（エラー）ならファイルを閉じてスキップ。
    result = sys_seek(fd, 16, SEEK_SET);
    if(result < 0){ sys_close(fd); continue; }
    // 16バイト目から16バイト（EXFID）を読み込み変数に保存。読み込んだのが16バイトでないならファイルを閉じてスキップ。
    result = sys_read(fd, &readexfid[0], 16);
    if(result != 16){ sys_close(fd); continue; }
    // 読み込んだものと渡されたEXFIDを比較。0（オーケー）なら処理中のファイルディスクリプタをグローバルFDに保存して関数を終了。
    if(memcmp(&readexfid[0], &exfid[0], 16) == 0){
      globalfd = fd;
      return 0;
    }
    // 見つからないならファイルを閉じる。次の処理へ。
    sys_close(fd);
  }
  // 何も見つからなかったら-1を返す。
  return -1;
}

// パスからEXFを探してリストを取得
int exefkit_getexflist(const char* path, char namelist[8][256]){
  // 検索に使用するファイルディスクリプタ。結果を保存する関数。
  int fd = -1;
  int result = -1;
  // ファイルタイプを保存する変数。
  unsigned long filetype;
  // ファイル数のカウントをする変数。
  int count = 0;
  // 検索パターンを保存する変数。渡されたパスから検索パターンを作成。
  char pattern[MAXPATHLEN];
  if(strlen(&path[0]) + strlen("*.exf") + 1 >= MAXPATHLEN) return -1;
  sprintf(&pattern[0], "%s%s", &path[0], "*.exf");
  // 最初のファイルを探す。
  result = sys_findfirst(&pattern[0], &fd, &namelist[count][0], &filetype);
  // 最初のファイルが見つかった（結果が0）なら。
  if(result == 0){
    // ファイル数を+1。
    count++;
    // 残りも探す。
    while (count < 8){
      result = sys_findnext(fd, &namelist[count][0], &filetype);
      if(result == 0){
        count++;
      }else{
        break;
      }
    }
  }
  // もしFDが0以上（使用中）なら、それを閉じてファイル数を返す。
  if (fd >= 0) sys_findclose(fd);
  return count;
}

// UTF32コードから文字を探す。
int exefkit_findchar(wchar_t code){
  // グローバルファイルディスクリプタが0未満（未初期化）なら-1を返して終了。
  if(globalfd < 0) return -1;
  // 結果を保存する変数。
  int result = -1;
  // EXFが含む全文字数を保存する変数。（2バイト）
  unsigned short total_chars = 0;
  // オフセット位置を12バイト目に設定。結果が0以下なら-1を返して終了。
  result = sys_seek(globalfd, 12, SEEK_SET);
  if(result < 0) return -1;
  // 12バイト目から2バイト（文字数）を読む。それを変数に保存。読み込んだのが2バイトでないなら-1を返して終了。
  result = sys_read(globalfd, &total_chars, 2);
  if(result != 2) return -1;

  // 二分探索処理。HIGHは文字数をintに変えて1を引いたもの。（インデックス番号用）
  int low = 0;
  int high = (int)total_chars - 1;
  // 見つかったインデックス番号を保存する変数。
  int found_index = -1;
  // LOWがHIGH以下な間繰り返す。
  while(low <= high){
    // 想定下辺と想定上辺の真ん中を出す。
    int mid = low + (high - low) / 2;
    // 今のUTFコードを格納する変数。
    unsigned int current_code = 0;
    // 真ん中のコードを取得。
    result = sys_seek(globalfd, 32 + (mid * 112), SEEK_SET);
    if(result < 0) return -1;
    result = sys_read(globalfd, &current_code, 4);
    if(result != 4) return -1;
    // 合ってたらループ終了。
    if(current_code == (unsigned int)code){
      found_index = mid;
      break;
    }
    // 今と比べて前後どっちにあるか確認して割り出す。
    if(current_code < (unsigned int)code){
      low = mid + 1;
    }else{
      high = mid - 1;
    }
  }
  // 終わったら見つかったインデックス番号を返す。
  return found_index;
}

int exefkit_drawfont16(int index, int basex, int basey, unsigned short color, int scale){
  if(globalfd < 0) return -1;
  if(index < 0) return -1;
  int result = -1;
  unsigned char width = 0;
  unsigned char bitmap[32];
  result = sys_seek(globalfd, 32 + (index * 112) + 8, SEEK_SET);
  if(result < 0) return -1;
  result = sys_read(globalfd, &bitmap[0], 32);
  if(result != 32) return -1;
  result = sys_seek(globalfd, 32 + (index * 112) + 4, SEEK_SET);
  if(result < 0) return -1;
  result = sys_read(globalfd, &width, 1);
  if(result != 1) return -1;
  set_pen(color);
  for(int y = 0; y < 16; y++){
    unsigned char left = bitmap[y * 2];
    unsigned char right = bitmap[y * 2 + 1];
    for(int x = 0; x < 8; x++){
      if(left & (0x80 >> x)){
        draw_rect(basex + x * scale, basey + y * scale, scale, scale);
      }
    }
    for(int x = 0; x < 8; x++){
      if(right & (0x80 >> x)){
        draw_rect(basex + (x+8) * scale, basey + y * scale, scale, scale);
      }
    }
  }
  return (int)width;
}

int exefkit_drawfont24(int index, int basex, int basey, unsigned short color, int scale){
  if(globalfd < 0) return -1;
  if(index < 0) return -1;
  int result = -1;
  unsigned char width = 0;
  unsigned char bitmap[72];
  result = sys_seek(globalfd, 32 + (index * 112) + 40, SEEK_SET);
  if(result < 0) return -1;
  result = sys_read(globalfd, &bitmap[0], 72);
  if(result != 72) return -1;
  result = sys_seek(globalfd, 32 + (index * 112) + 5, SEEK_SET);
  if(result < 0) return -1;
  result = sys_read(globalfd, &width, 1);
  if(result != 1) return -1;
  set_pen(color);
  for(int y = 0; y < 24; y++){
    unsigned char left = bitmap[y * 3];
    unsigned char middle = bitmap[y * 3 + 1];
    unsigned char right = bitmap[y * 3 + 2];
    for(int x = 0; x < 8; x++){
      if(left & (0x80 >> x)){
        draw_rect(basex + x * scale, basey + y * scale, scale, scale);
      }
    }
    for(int x = 0; x < 8; x++){
      if(middle & (0x80 >> x)){
        draw_rect(basex + (x+8) * scale, basey + y * scale, scale, scale);
      }
    }
    for(int x = 0; x < 8; x++){
      if(right & (0x80 >> x)){
        draw_rect(basex + (x+16) * scale, basey + y * scale, scale, scale);
      }
    }
  }
  return (int)width;
}

// sizeは16か24の倍数にします。文字はL"あいう"のように指定します。
void exefkit_drawtext(const wchar_t *text, int basex, int basey, int size, unsigned short color){
  if(text == NULL) return;
  if(globalfd < 0) return;
  if (size % 16 != 0 && size % 24 != 0) {
    return;
  }
  int current = 0;
  int allsize = 0;
  int nowsize = 0;
  while(text[current] != L'\0'){
    int index = exefkit_findchar(text[current]);
    if(size % 24 == 0){
      nowsize = exefkit_drawfont24(index, basex + allsize, basey, color, size / 24);
      if(nowsize == -1){ allsize = allsize + 24 * (size / 24); current++; continue; };
      allsize = allsize + nowsize * (size / 24);
    }else if (size % 16 == 0){
      nowsize = exefkit_drawfont16(index, basex + allsize, basey, color, size / 16);
      if(nowsize == -1){ allsize = allsize + 16 * (size / 16); current++; continue; };
      allsize = allsize + nowsize * (size / 16);
    }
    current++;
  }
}

void exefkit_deinit(){
    if(globalfd < 0) return;
    sys_close(globalfd);
    globalfd = -1;
    return;
}
