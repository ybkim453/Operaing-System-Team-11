# Operating System Team 11 - Mini OS

## 빌드 환경 (로컬 테스트를 위함)

### Windows (WSL2 + Ubuntu 22.04)

PowerShell을 **관리자 권한**으로 실행 후 Ubuntu 22.04 설치:
```powershell
wsl --install -d Ubuntu-22.04
```
재부팅 후 Ubuntu가 자동 실행되면 사용자 계정 설정 완료.

WSL 진입 후 빌드 도구 및 git 설치:
```powershell
wsl
```
```bash
sudo apt update
sudo apt install build-essential git
```

### macOS

Xcode Command Line Tools 설치 (gcc, make 포함):
```bash
xcode-select --install
```

---

### 프로젝트 클론 및 실행

```bash
git clone https://github.com/ybkim453/Operaing-System-Team-11.git
cd Operaing-System-Team-11

# 빌드
make

# 실행
./mini_os

# 빌드 + 실행 한 번에
make run

# 빌드 결과물 초기화
make clean
```

## 프로젝트 구조

```
.
├── include/
│   ├── vfs.h          전역 구조체 및 상수 정의 (VNode, VFS, UserDB, ...)
│   ├── vfs_core.h     시스템 초기화 함수 선언
│   ├── path_stack.h   경로 스택 구조 및 함수 선언
│   ├── command.h      명령어 함수 선언
│   └── parser.h       파싱 관련 함수 선언
├── src/
│   ├── main.c         메인 루프, 파일시스템 save/load 호출
│   ├── vfs_core.c     VFS 초기화/해제, 파일시스템 직렬화, 유저 관리
│   ├── path_stack.c   스택 구현 (push/pop/is_empty)
│   ├── command.c      공통 유틸리티 (MakeDir, IsPermission, update_node_time)
│   ├── parser.c       입력 파싱 및 명령어 디스패치
│   ├── ls.c
│   ├── cd.c
│   ├── pwd.c
│   ├── mkdir.c        -p 옵션, 다중 디렉토리 pthread 병렬 생성
│   ├── cat.c
│   ├── rm.c
│   ├── mv.c           이름 변경 및 이동 (신규 구현)
│   ├── grep.c
│   ├── chown.c
│   └── user.c
├── Makefile
└── README.md
```

## 구현 명령어

### 필수 (모든 조 공통)
| 명령어 | 옵션 | 설명 |
|--------|------|------|
| `ls` | `-a` `-l` `-al` | 디렉토리 목록 |
| `cd` | `.` `..` `/` | 디렉토리 이동 |
| `mkdir` | `-p` `-m` | 디렉토리 생성, 2개 이상은 pthread 병렬 생성 |
| `cat` | `>` `-n` | 파일 읽기/생성 |
| `pwd` | | 현재 경로 출력 |

### 홀수조 전용
| 명령어 | 옵션 | 설명 |
|--------|------|------|
| `chown` | `-R` | 소유자/그룹 변경 |
| `grep` | `-n` `-i` `-v` `-o` | 파일 내용 검색 |
| `mv` | | 파일/디렉토리 이동 및 이름 변경 |
| `rm` | `-r` `-f` `-v` | 삭제 |

### 기타
| 명령어 | 설명 |
|--------|------|
| `useradd` / `userdel` / `userlist` | 사용자 관리 (chown 기능 test를 위해 넣음) |


## 테스트 시나리오

### 1. pwd / mkdir / cd / ls

```
pwd
mkdir test1 test2 test3
ls
mkdir -p test1/test1-1/test1-2 test2/test2-1
cd test1
pwd
ls
cd test1-1
ls
cd /
pwd
```

### 2. cat — 파일 생성 및 읽기

```
cat > hello.txt
Hello World
Hello World2
^D
cat hello.txt
cat -n hello.txt
```

```
cat > notes.txt
first note
second note
^D
cat notes.txt
```

### 3. ls 옵션

```
ls -a
ls -l
ls -al
```

### 4. mv — 이름 변경 및 이동

```
mv hello.txt renamed.txt
cat renamed.txt
ls
mv renamed.txt test3
cd test3
ls
mv renamed.txt /hello.txt
cd /
ls
```

### 5. rm — 단일 / 다중 / 재귀

```
rm hello.txt
ls
mkdir tmp1 tmp2 tmp3
ls
rm -rv tmp1 tmp2 tmp3
ls
mkdir -p test4/test4-1
cat > test4/test4-1/file.txt
content1
^D
rm -r test4
ls
```

### 6. grep — 단일 파일 / 다중 파일 / 옵션

```
grep test notes.txt
grep note notes.txt
grep -n note notes.txt
grep -i FIRST notes.txt
grep -v note notes.txt
grep -o note notes.txt
```

### 7. chown — 소유자 변경

```
useradd alice
userlist
cat > owned.txt
owner test
^D
ls -l
chown alice owned.txt
ls -l
mkdir -p project/src
chown -R alice project
ls -l
cd project
ls -l
```

### 8. 멀티스레딩 확인 (mkdir / rm / grep)

```
mkdir a b c d e
ls
rm -rv a b c d e
ls
cat > file1.txt
apple banana
^D
cat > file2.txt
banana cherry
^D
cat > file3.txt
cherry apple
^D
grep apple file1.txt file2.txt file3.txt
grep -n banana file1.txt file2.txt file3.txt
```

### 9. 종료

```
exit
```