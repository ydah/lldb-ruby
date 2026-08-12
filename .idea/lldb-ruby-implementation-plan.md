# lldb-ruby Binding Hardening 作業手順書

- 対象: [`ydah/lldb-ruby`](https://github.com/ydah/lldb-ruby)
- 基準コミット: `632f2fe42e1cc9d157dd6ae09ee6f1693baea003`
- 対応設計: [`lldb-ruby-binding-design.md`](./lldb-ruby-binding-design.md)
- 作成日: 2026-08-12

## 1. 進め方

一つの巨大PRにしない。次の依存順で分ける。

```text
PR-01 build discovery / capability / LLDB version matrix
  ├─ PR-02 enum / marshalling / bounded buffers
  ├─ PR-03 error fidelity / C++ exception barrier
  │    └─ PR-04 native handle lifecycle / Value execution context
  │          └─ PR-05 launch semantics
  ├─ PR-06 FileSpec / buffer / sentinel
  └─ PR-07 binding parity / package hardening

PR-03 + PR-04 + PR-06
  └─ PR-08 blocking/GVL

PR-01 + PR-03 + PR-04
  └─ PR-09 event bindings
       └─ PR-10 option objects
            └─ PR-11 API cluster completion
```

各PRは、原則として次の順でcommitする。

1. 失敗するtestまたはCI job
2. C header / C++ wrapper
3. FFI
4. Ruby object API
5. RBS
6. documentation / changelog

C ABI、FFI、RBSの変更を別PRに分離しない。

## 2. 共通ルール

### 2.1 bindingsの境界

実装中に次が必要になった場合は、その場でcoreへ入れない。

- polling loop
- retry
- background thread
- auto-continue
- implicit timeout
- 利用者が指定していないlaunch flag
- LLDB eventの自動dispatch

必要性をREADMEのexampleで満たせない場合だけ、別設計として検討する。

### 2.2 public APIの追加条件

新規methodは同じPRで次を揃える。

- `lldb_wrapper.h`
- `lldb_wrapper.cpp`
- `ffi_bindings.rb`
- Ruby wrapper
- RBS
- spec
- capabilityまたは最低LLDB 14でのcompile確認
- ownershipの記述

### 2.3 invalid object

`valid?`だけは例外を送出しない。それ以外のobject methodは、closedまたはinvalidなら`InvalidObjectError`系を送出する方向へ揃える。

この変更は互換性へ影響するため、既存methodを一括変更せず、PR-04でまとめる。

### 2.4 testのskip

次のようなskipは避ける。

```ruby
skip "not available" if value.nil?
```

そのAPIを保証するfixtureなら、`nil`はfailureにする。platform上の真の制約だけをskipする。

許容例:

```ruby
skip "hardware watchpoints are unavailable on this runner" \
  unless process.num_supported_hardware_watchpoints.positive?
```

optional APIはskipせず、capabilityに応じた期待値を明示する。

## 3. 作業前のbaseline

### 3.1 branchと状態確認

```bash
git switch main
git pull --ff-only
git rev-parse HEAD
git status --short
git switch -c harden/binding-foundation
```

基準コミットと異なる場合は、設計で参照している箇所を再確認する。

### 3.2 現行test

```bash
bundle install
bundle exec rake clean
bundle exec rake compile
bundle exec rspec
bundle exec rake steep
```

結果をPR description用に記録する。

### 3.3 gem package

```bash
rm -rf tmp/gem-home
mkdir -p tmp/gem-home

gem build lldb.gemspec
GEM_HOME="$PWD/tmp/gem-home" \
GEM_PATH="$PWD/tmp/gem-home" \
  gem install ./lldb-*.gem --no-document

env -u RUBYLIB \
  GEM_HOME="$PWD/tmp/gem-home" \
  GEM_PATH="$PWD/tmp/gem-home" \
  ruby -e 'require "lldb"; puts LLDB::VERSION'
```

失敗しても最初からbuild layoutを推測で直さない。stdout/stderr、生成物、install先を記録してPR-01のtestに固定する。

---

## PR-01: LLDB discovery、compile probe、capability、version matrix

### 4. 目的

- READMEに書かれたLLDB 14以上という契約をCIで検証する。
- `LLDB_DIR`を実装する。
- version macroによるwatchpoint判定をcompile probeへ置き換える。
- `APISupport`をwrapper symbolの有無ではなくcapabilityへ接続する。
- built gemをsource treeなしでloadできることを保証する。

### 5. 変更対象

- `ext/lldb/extconf.rb`
- `ext/lldb/lldb_wrapper.h`
- `ext/lldb/lldb_wrapper.cpp`
- `lib/lldb/ffi_bindings.rb`
- `lib/lldb/api_support.rb`
- `lib/lldb/watchpoint.rb`
- `spec/lldb/api_support_spec.rb`
- `spec/lldb/watchpoint_spec.rb`
- `.github/workflows/ci.yml`
- 必要なら`spec/support/`
- `README.md`
- `CHANGELOG.md`

### 6. 手順

#### 6.1 discoveryの入力を定義する

`extconf.rb`で次を受け取る。

```text
--with-lldb-dir
--with-lldb-include
--with-lldb-lib
LLDB_DIR
```

優先順位をtest可能なmethodへ切り出す。

期待する順序:

1. explicit include/lib
2. explicit dir
3. `LLDB_DIR`
4. unversioned `llvm-config`
5. version付き`llvm-config-*`
6. platform prefix
7. `/usr/lib/llvm-*`

#### 6.2 hard-coded version列挙を廃止する

次のような固定列挙を削除する。

```ruby
find_executable("llvm-config-18") ||
find_executable("llvm-config-17") ||
...
```

PATH上の`llvm-config-*`を列挙し、versionを数値として比較する。選択した実行ファイル、include、libをbuild logへ出す。

#### 6.3 C++ compile/link probeを作る

同じcompiler、include、libを使い、次を検証する。

```cpp
#include <lldb/API/LLDB.h>

int main() {
  lldb::SBDebugger debugger;
  return debugger.IsValid() ? 0 : 0;
}
```

compile成功だけでなく`-llldb`までlinkする。

失敗messageには次を含める。

- 試したcompiler
- include path
- lib path
- `llvm-config`
- 明示指定の方法

#### 6.4 capability probeを作る

少なくともwatchpoint access predicateをprobeする。

```cpp
auto reads = &lldb::SBWatchpoint::IsWatchingReads;
auto writes = &lldb::SBWatchpoint::IsWatchingWrites;
```

成功した場合だけ生成headerへdefineする。

```c
#define LLDB_RUBY_HAVE_WATCHPOINT_ACCESS_KIND 1
```

`LLDB_VERSION_MAJOR`による条件分岐は削除する。

#### 6.5 wrapper metadataを追加する

C ABI:

```c
uint32_t lldb_wrapper_abi_version(void);
const char *lldb_wrapper_build_lldb_version(void);
const char *lldb_wrapper_runtime_lldb_version(void);
int lldb_wrapper_has_capability(uint32_t capability);
```

最初のABI versionは`1`とする。

Ruby:

```ruby
LLDB::Native.wrapper_abi_version
LLDB::Native.build_lldb_version
LLDB::Native.runtime_lldb_version
LLDB::APISupport.feature_supported?(:watchpoint_access_kind)
```

既存module名を維持する場合、`FFIBindings`内に置いてよい。public nameを増やすこと自体を目的にしない。

load順序は次にする。

1. 解決したwrapper pathを最小の`BootstrapBindings`でloadする。
2. `lldb_wrapper_abi_version`だけをattachする。
3. Ruby側の期待ABIと比較する。
4. 一致した場合だけ本体`attach_function`群を定義する。

metadata symbol欠落またはABI不一致は`IncompatibleWrapperError`へ変換し、load path、期待値、実値をmessageへ含める。古いwrapperで本体symbolのattachに失敗してから気づく構造を残さない。

#### 6.6 watchpoint C ABIを三値からstatusへ変える

最低限、未対応をboolと同じ戻り値へ詰め込まない。

```c
lldb_ruby_status_t
lldb_watchpoint_is_watching_reads(
  lldb_watchpoint_t watchpoint,
  int *result
);
```

LLDB 14〜16:

```text
status = UNSUPPORTED
```

LLDB 17以降:

```text
status = OK
result = 0 or 1
```

Rubyでは未対応時に`UnsupportedAPIError`を送出する。

#### 6.7 `APISupport`を修正する

- `FEATURES`をoptional featureだけへ絞る。
- `feature_supported?`はnative capabilityを呼ぶ。
- `method_available?`へdeprecation warningを付けるか、privateへ移す。
- `supported_features` / `unsupported_features`をcapabilityに基づかせる。

#### 6.8 CI matrixを追加する

PR必須:

```text
Linux / LLDB 14 / Ruby 3.0
Linux / LLDB 16 / current Ruby
Linux / LLDB 17 / current Ruby
Linux / current stable LLDB / current Ruby
macOS / Homebrew LLVM / current Ruby
```

LLDB 16と17は同じjobの「versionだけ違うmatrix」にする。watchpoint API境界のtestを明示する。

#### 6.9 package smokeを追加する

CIで次を実行する。

```bash
gem build lldb.gemspec
GEM_HOME="$RUNNER_TEMP/gem-home" \
GEM_PATH="$RUNNER_TEMP/gem-home" \
  gem install ./lldb-*.gem --no-document

cd "$RUNNER_TEMP"
GEM_HOME=... GEM_PATH=... ruby -e '
  require "lldb"
  puts LLDB::FFIBindings.lldb_wrapper_abi_version
'
```

source checkoutの`lib`を誤ってloadしていないことを確認する。

#### 6.10 platform contractを一致させる

正式対象をLinuxとmacOSへ限定する。現行のRuby側だけにある`.dll`探索分岐は、Windows対応が存在するように見えるため削除するか、build時とload時の両方で明示的な`UnsupportedPlatformError`に統一する。

Windows対応は次を同じ変更で用意できるまで追加しない。

- exported symbol attribute
- MSVCまたはMinGW向けbuild
- LLDB discovery
- `.dll`のinstall layout
- CI integration test

READMEのsupport表も同じ契約へ更新する。

### 7. PR-01 test

#### 7.1 unit

```ruby
expect(APISupport.feature_supported?(:unknown)).to be(false)
expect(wrapper_abi_version).to eq(1)
expect(runtime_lldb_version).not_to be_empty
```

metadata symbolのないfixture libraryとABI versionが異なるfixture libraryをloadし、本体attach前に`IncompatibleWrapperError`になることも確認する。

#### 7.2 LLDB 14・16

```ruby
expect(
  APISupport.feature_supported?(:watchpoint_access_kind)
).to be(false)

expect { watchpoint.watching_reads? }
  .to raise_error(LLDB::UnsupportedAPIError)
```

hardware watchpointを作れないrunnerでは、C ABIを直接testするfixture shimを用意してcapabilityだけを検証する。

#### 7.3 LLDB 17以降

```ruby
expect(
  APISupport.feature_supported?(:watchpoint_access_kind)
).to be(true)
```

実watchpointを作れるrunnerではpredicateがbooleanを返すことも確認する。

#### 7.4 discovery

一時directoryにfake `llvm-config`を置き、選択順をunit testする。link自体はintegration jobで確認する。

```bash
LLDB_DIR=/usr/lib/llvm-14 bundle exec rake compile
```

### 8. PR-01 commit案

1. `Add package and LLDB boundary build checks`
2. `Implement LLDB discovery and compile probes`
3. `Expose wrapper ABI metadata and capabilities`
4. `Fix watchpoint feature detection`
5. `Document supported LLDB discovery paths`

### 9. PR-01完了条件

- LLDB 14、16、17、現行versionでcompileする。
- LLDB 15/16で`IsWatchingReads/Writes`をcompileしない。
- LLDB 14〜16でpredicateが`true`にならない。
- `LLDB_DIR`が実際に使われる。
- built gemを空のGEM_HOMEへinstallしてloadできる。
- ABI不一致wrapperを本体FFI attach前に拒否できる。
- `APISupport`が`respond_to?`へ依存しない。
- Linux/macOS以外を誤って対応済みと表示しない。

---

## PR-02: enum、marshalling、bounded CString

### 10. 目的

architectureを大きく変える前に、現行APIの明確な誤動作を修正する。

### 11. 変更対象

- `ext/lldb/lldb_wrapper.h`
- `ext/lldb/lldb_wrapper.cpp`
- `lib/lldb/types.rb`
- `lib/lldb/launch_info.rb`
- `lib/lldb/process.rb`
- `lib/lldb/ffi_bindings.rb`
- `spec/lldb/types_spec.rb`
- `spec/lldb/launch_info_spec.rb`
- `spec/lldb/process_spec.rb`
- `spec/fixtures/`
- `CHANGELOG.md`

### 12. 手順

#### 12.1 enumを最低LLDB 14へ合わせる

追加・訂正:

```ruby
module StopReason
  PROCESSOR_TRACE = 11
  FORK = 12
  VFORK = 13
  VFORK_DONE = 14
end

module ValueType
  VARIABLE_THREAD_LOCAL = 8
end

module SymbolContextItem
  EVERYTHING = 0x7f
  VARIABLE = 1 << 7
end
```

`NAMES`も同じcommitで更新する。

C headerに複製したenum macroを削除し、C++側は上流enumを直接使う。

```cpp
return static_cast<int>(lldb::eStateInvalid);
```

#### 12.2 `NativeStringArray`を作る

内部classまたはmodule helperとして実装する。

保持するもの:

```text
@strings
@string_pointers
@pointer_array
```

native callの終了までhelperをlocal variableで保持する。

`LaunchInfo.new(args)`と`set_environment`をこれへ移す。

#### 12.3 environment semanticsを修正する

```ruby
set_environment(nil)                  # no-op
set_environment({}, append: true)     # no additions
set_environment({}, append: false)    # replace with empty
```

C++側の`SBEnvironment`生成と`SetEnvironment`が空環境を受け取れることをtestする。

#### 12.4 NULを検証する

次を`ArgumentError`にする。

- argv要素のNUL
- environment keyのNUL
- environment keyの`=`
- environment valueのNUL
- C stringとして渡すpath/name/conditionのNUL

全String APIへ一度に適用せず、共通helperを作り、変更したmethodから使う。

#### 12.5 `ReadCStringFromMemory`をboundedにする

現行は戻り値を無視しているため修正する。

```ruby
bytes_read = native_call(...)
error.raise_if_error!

bytes = buffer.read_string_length([bytes_read, max_size].min)
bytes = bytes.delete_suffix("\0")
```

`bytes_read`が末尾NULを含むかはLLDB 14と現行versionのintegration testで固定する。実装を仮定に合わせず、test結果に合わせる。

#### 12.6 stdinとmemory writeをpointer化する

`Process#put_stdin`のFFI引数を`:string`から`:pointer`へ変更する。Ruby Stringを`put_bytes`で明示copyし、`bytesize`を渡す。

`write_memory`も同じhelperへ揃える。

### 13. PR-02 test

#### enum

```ruby
expect(StopReason.name(11)).to eq("processor trace")
expect(ValueType.name(8)).to eq("thread local")
expect(SymbolContextItem::EVERYTHING).to eq(0x7f)
expect(SymbolContextItem::EVERYTHING & SymbolContextItem::VARIABLE).to eq(0)
```

#### GC

```ruby
previous = GC.stress
GC.stress = true
begin
  info = LLDB::LaunchInfo.new(["a", "b", "c"])
  info.set_environment({"A" => "B"}, append: false)
  # launchまたはnative inspectionで値を確認
ensure
  GC.stress = previous
end
```

unit testではtest-only C ABIでargv/envpをcopyして返す方法を使ってもよい。debug process起動だけに依存させない。

#### empty environment

fixtureが環境変数一覧を出力するか、特定keyの有無を返すようにする。

#### binary

```ruby
payload = "a\0b".b
expect(process.put_stdin(payload)).to eq(payload.bytesize)
expect(process.write_memory(address, payload)).to eq(payload.bytesize)
```

#### CString

- 先頭NUL
- 中間NUL
- max_size - 1
- max_sizeぴったり
- NULなし
- error
- 0 byte

### 14. PR-02 commit案

1. `Align public enums with LLDB 14`
2. `Keep FFI string arrays alive during native calls`
3. `Preserve empty environments`
4. `Bound C string reads and make byte buffers explicit`

### 15. PR-02完了条件

- minimum LLDB 14のenumが揃う。
- `SymbolContextItem::EVERYTHING`がvariable bitを含まない。
- `GC.stress`下でargv/envpが壊れない。
- 空環境置換が可能である。
- binary stdin/memoryがNULで切れない。
- CString readがallocation外へ進まない。

---

## PR-03: error fidelityとoperation status

### 16. 目的

LLDBが返した`SBError`と`ReturnStatus`を保持し、成功を推測しない。

### 17. 変更対象

- `ext/lldb/lldb_wrapper.h`
- `ext/lldb/lldb_wrapper.cpp`
- `lib/lldb/ffi_bindings.rb`
- `lib/lldb/error.rb`
- `lib/lldb/process.rb`
- `lib/lldb/thread.rb`
- `lib/lldb/value.rb`
- `lib/lldb/command_return_object.rb`
- `lib/lldb.rb`
- 関連spec / RBS / CHANGELOG

### 18. 手順

#### 18.1 status enumを追加する

PR-01でwatchpoint用に導入済みなら共通化する。

```c
LLDB_RUBY_STATUS_OK
LLDB_RUBY_STATUS_INVALID_ARGUMENT
LLDB_RUBY_STATUS_INVALID_HANDLE
LLDB_RUBY_STATUS_UNSUPPORTED
LLDB_RUBY_STATUS_LLDB_ERROR
LLDB_RUBY_STATUS_INTERNAL_ERROR
```

Rubyに`Native.check_status`相当の一箇所を作る。

#### 18.2 C++ exception barrierを導入する

`extern "C"`の外へC++例外を出さない。wrapper内部にthread-localなerror stateと共通guardを作る。error stateは固定長`char` bufferと数値codeで構成し、`std::bad_alloc`のcatch節でもallocationしない。

```text
lldb_wrapper_last_error_message
lldb_wrapper_clear_last_error
```

guardは少なくとも次を捕捉する。

- `std::bad_alloc`
- `std::exception`
- その他の例外

扱いを戻り値ごとに分ける。

- status API: `INTERNAL_ERROR`を返す。
- pointer API: `nullptr`を返し、thread-local errorを設定する。
- scalar API: 型ごとの安全なsentinelを返し、thread-local errorを設定する。
- destroy API: 例外を記録するが外へ出さない。explicit `close`は記録を確認して`InternalBindingError`にできるが、finalizerは例外を抑止する。

通常exportはentry時にerror stateをclearする。Ruby object層はnative callを`Native.invoke(operation) { ... }`のような一箇所へ通し、call直後に同じRuby thread上でwrapper errorを確認する。これにより0が正しいgetterでも内部例外と区別し、古いerrorを次のcallへ持ち越さない。metadata/error取得関数自身はerror stateをresetしない`noexcept`実装にする。

test buildだけで有効になるthrowing exportを用意し、各return categoryでRuby processがabortせず`InternalBindingError`を得ることを確認する。最終的に全exportがguard経由であることをPR-07のcheckerで検査する。

#### 18.3 `SBError::GetType`をbindする

```text
lldb_error_get_type
```

Ruby:

```ruby
Error#code
Error#type
Error#type_name
Error#message
```

同時にLLDB 14の`ErrorType`定数とname mappingを追加し、PR-07のconstant parity対象へ入れる。`error_code`はaliasとして残す。

#### 18.4 `OperationError`を追加する

例外が保持するもの:

```text
operation
Error object
code
error_type
message
```

例外message例:

```text
LLDB operation 'process.continue' failed (type=POSIX, code=3): ...
```

messageのformatに依存するtestは最小にし、属性を直接testする。

#### 18.5 initialize errorを保持する

C++:

```cpp
lldb::SBError error = lldb::SBDebugger::InitializeWithErrorHandling();
```

Ruby:

```ruby
LLDB.initialize
# failure => OperationError(operation: "lldb.initialize")
```

初期化済みの場合の冪等性は維持する。

#### 18.6 process操作を変更する

対象:

```text
continue
stop
kill
detach
destroy
signal
deallocate_memory
num_supported_hardware_watchpoints
```

`SBError`をout parameterへcopyし、statusを返す。

Rubyは成功時`true`、失敗時`OperationError`とする。既存の`false`を使ったfailure branchはmigration noteへ記載する。

#### 18.7 stepをerror overloadへ変更する

LLDB 14でerror overloadがあるもの:

- `StepOver(RunMode, SBError&)`
- `StepOut(SBError&)`
- `StepInstruction(bool, SBError&)`
- `RunToAddress(addr, SBError&)`

`StepInto`はtarget name、end line、error、run modeを取るoverloadを使う。invalid line sentinelを上流定数から渡す。

Ruby API例:

```ruby
thread.step_over(run_mode: LLDB::RunMode::ONLY_DURING_STEPPING)
thread.step_into(
  target_name: nil,
  end_line: LLDB::INVALID_LINE_NUMBER,
  run_mode: LLDB::RunMode::ONLY_DURING_STEPPING
)
```

既存引数なしcallはnative defaultと等価になるようmappingする。`RunMode`定数は上流LLDB 14の値と照合し、PR-07のconstant parity対象へ入れる。

#### 18.8 pure voidを正す

`Process#send_async_interrupt`は`nil`を返す。成功boolを作らない。

#### 18.9 Value数値変換をerror overloadへ変更する

```cpp
GetValueAsSigned(error, fail_value)
GetValueAsUnsigned(error, fail_value)
```

Rubyではerror時に`OperationError`を送出する。正しい値0は0として返る。

#### 18.10 command statusを公開する

`SBCommandReturnObject::GetStatus`をbindする。LLDB 14の`ReturnStatus`定数とname mappingも同じcommitで追加する。

Ruby:

```ruby
result.status
result.succeeded?
result.has_result?
```

C wrapperの`HandleCommand`はsuccess boolへ縮退せず、上流`ReturnStatus`を返すか`CommandReturnObject`に保持する。

### 19. PR-03 test

#### C++ exception boundary

- status return
- pointer return
- scalar return
- destroy

のtest-only throwing exportを呼び、Ruby processがabortせず、operation名とnative messageを持つ`InternalBindingError`になることを確認する。別Ruby threadでもerror stateが混線しないことを確認する。

#### process error

終了済みprocessへのcontinue等、安定して失敗する操作を使う。

```ruby
expect { process.continue }
  .to raise_error(LLDB::OperationError) { |e|
    expect(e.operation).to eq("process.continue")
    expect(e.error).to be_a(LLDB::Error)
    expect(e.error).to be_fail
  }
```

#### step error

suspended threadまたはinvalid状態でのstepを使う。platform差が大きい場合はtest-only wrapperで`SBError` copyをunit testし、integrationでは属性を緩く確認する。

#### numeric zero

値0の変数と変換不能な値を別fixtureにする。

```ruby
expect(zero.value_as_signed).to eq(0)
expect { invalid.value_as_signed }.to raise_error(...)
```

#### command status

- 成功・resultあり
- 成功・resultなし
- failure
- continuing

### 20. PR-03 commit案

1. `Introduce native operation statuses`
2. `Stop C++ exceptions at the C ABI boundary`
3. `Preserve SBError type and operation context`
4. `Use InitializeWithErrorHandling`
5. `Preserve process operation errors`
6. `Use error-reporting thread step overloads`
7. `Preserve value conversion and command statuses`

### 21. PR-03完了条件

- process failureのcode/type/messageをRubyで取得できる。
- stepが無条件に`true`を返さない。
- 変換失敗と整数0を区別できる。
- initialization failureを捨てない。
- `SendAsyncInterrupt`が成功boolを捏造しない。
- commandの`ReturnStatus`を取得できる。
- C++例外がC ABIを越えず、Rubyの`InternalBindingError`として観測できる。
- thread-local wrapper errorがRuby thread間で混線しない。

---

## PR-04: native handle、close、Debugger Context

### 22. 目的

native objectをGC timingだけに任せず、二重releaseとDebugger終了後の利用を防ぐ。

### 23. 変更対象

- 新規 `lib/lldb/native_handle.rb`
- 新規 `lib/lldb/context.rb`
- 全native wrapper class
- `lib/lldb/value.rb`
- `spec/lldb/value_spec.rb`
- `lib/lldb.rb`
- lifecycle spec
- RBS / README / CHANGELOG

### 24. 手順

#### 24.1 `NativeHandle`をtest-firstで作る

必要API:

```ruby
handle.to_ptr
handle.close
handle.closed?
```

test用release procで呼び出し回数を数える。

必須case:

- explicit close 1回
- close 2回
- close後GC
- GCだけ
- null pointer
- close後to_ptr
- 二つのRuby threadから同時にclose

release済み判定とpointer swapはMutexで直列化し、native release functionを最大一回だけ呼ぶ。

#### 24.2 common object behaviorを作る

mixinまたはbase classで次を統一する。

```text
close
closed?
to_ptr
valid?
ensure_open!
```

継承を強制してLLDB class hierarchyと誤認させるより、内部compositionを優先する。

#### 24.3 `Context`を導入する

Debugger作成時にContextを作り、子objectへ渡す。

```ruby
debugger.context.equal?(target.context)
target.context.equal?(process.context)
```

Contextは`open`、`closing`、`closed`の状態とMutexを持つ。native handleをweak registryへ登録し、Contextからwrapperを強参照しない。Ruby 3.0でのWeakMap/WeakRef挙動をunit testしてから実装方式を固定する。

public accessorにする必要はない。specではinternal helperから確認してよい。

#### 24.4 親から子へのcacheを削除する

削除:

- `Debugger#@targets`
- `Target#@breakpoints`
- `Target#@watchpoints`
- `Target#@process`

`delete_*`後のRuby cache更新処理も削除する。

#### 24.5 Context-aware validを実装する

```ruby
def valid?
  return false if closed?
  return false if context&.closed?
  native_valid?
end
```

通常methodは`ensure_valid!`を通す。

#### 24.6 `Value`のexecution contextをnativeから取得する

Context導入後に、次を同じPRでbindする。

```text
lldb_value_get_target
lldb_value_get_process
lldb_value_get_thread
lldb_value_get_frame
```

Ruby API:

```text
Value#target
Value#process
Value#thread
Value#frame
```

返却objectには元のValueと同じContextを渡し、ownerとして元のValueを保持する。native objectの同一性は`SBValue::Get*`から得て、Rubyの親objectをduck typingで探索しない。

`Value#watch`は`Value#target`を使う。Frame由来Valueとnested Valueの両方をtestし、hardware watchpointを作れない環境でもtarget/context解決までは検証する。

#### 24.7 Debugger close

`Debugger#close`は次を行う。

1. Context lock内で`open`から`closing`へ遷移する。
2. weak registryからliveな子handleをsnapshotし、Debugger以外を先にcloseする。
3. Debuggerの`NativeHandle#close`を一度呼ぶ。release functionである`lldb_debugger_destroy`が`SBDebugger::Destroy`とC++ wrapperのdeleteをまとめて行う。
4. `ensure`でContextを`closed`へし、open debugger countを減らす。
5. explicit closeでnative内部errorがあれば、状態をclosedにした後で`InternalBindingError`を送出する。

`SBDebugger::Destroy`をRuby側から別途呼んでからhandleをreleaseしてはいけない。子objectは以後`ClosedObjectError`になる。

#### 24.8 runtime lifecycle

`LLDB.initialize`と`terminate`をMutexで保護する。

```ruby
LLDB.terminate
# open debuggerあり => LifecycleError
```

`at_exit`はopen debugger countが0の場合だけterminateする。

#### 24.9 `delete`との区別をtestする

```ruby
bp = target.breakpoint_create_by_name("main")
bp.close
expect(target.num_breakpoints).to eq(1)

bp2 = target.find_breakpoint_by_id(...)
bp2.delete
expect(target.num_breakpoints).to eq(0)
```

### 25. PR-04 test

- 全classの`close`が冪等
- concurrent closeでもreleaseは一回
- finalizerとの二重releaseなし
- Debugger closeでは子handleが先、Debugger releaseが最後
- childがparent/contextを保持
- parentからchildへのcycleなし
- Debugger close後のTarget/Process/Thread/Frame/Value操作が失敗
- Frame由来Valueとnested Valueの`target/process/thread/frame`が同じContextを持つ
- `Value#watch`が親objectの`respond_to?`探索へ依存しない
- `valid?`だけはfalseを返す
- `LLDB.terminate`がopen debuggerを見逃さない
- 100回以上のinitialize/create/close/terminate
- `GC.start`を途中に挟む
- `GC.stress`

ASan jobはこのPR以降に有効化する。

### 26. PR-04 commit案

1. `Add idempotent native handles`
2. `Share debugger contexts across binding objects`
3. `Remove parent-to-child wrapper caches`
4. `Bind SBValue execution context and fix watch ownership`
5. `Add deterministic close semantics`
6. `Guard LLDB termination with open debugger state`

### 27. PR-04完了条件

- serial/concurrent/finalizerの全経路で二重releaseしない。
- explicit closeできる。
- Debugger closeが子handleを先にdrainし、`lldb_debugger_destroy`を一度だけ呼ぶ。
- Debugger close後の子object利用を拒否する。
- 親子cycleを持たない。
- logical deleteとwrapper closeの意味が分離している。
- Frame由来Valueからnative Targetへ到達でき、`Value#watch`のContextが正しい。
- lifecycle stress testがASanで通る。

---

## PR-05: `Target#launch`を直接bindingへ戻す

### 28. 目的

起動後の制御方針をcore bindingsから除く。

### 29. 前提

PR-03とPR-04がmerge済みであること。errorとlifecycleが定まる前にlaunch behaviorを変えない。

### 30. 手順

#### 30.1 現行挙動をtestで記録する

互換性変更前のtestを別fileへ置き、何が消えるかを明示する。

- implicit `STOP_AT_ENTRY`
- wait loop
- auto-continue
- 10秒timeout

このtestは新挙動では削除するのではなく、migration documentationの根拠として残す。spec自体は新しい期待へ書き換える。

#### 30.2 `Target#launch`からpolicyを削除する

削除:

```text
launch_info.launch_flags = STOP_AT_ENTRY
wait_for_process_stop
num_breakpointsによるcontinue
Timeout
sleep
```

追加可能な明示keyword:

```ruby
launch_flags:
```

省略時は`SBLaunchInfo`の初期値を変更しない。

#### 30.3 `launch_with_info`を基準APIにする

`launch`は`LaunchInfo`を組み立てた後に`launch_with_info`へ委譲する。

```ruby
def launch(...)
  info = LaunchInfo.new(args)
  ...
  launch_with_info(info)
end
```

`launch_with_info`はnative call一回とerror変換だけを行う。

#### 30.4 exampleをevent API前提にしない

PR-09前にmergeする場合、READMEでは次を明記する。

- synchronous/asyncの挙動はLLDBに従う。
- 停止待ちは利用側責務。
- 当面はstateを明示確認できるが、core helperは提供しない。
- PR-09後にlistener exampleへ更新する。

#### 30.5 release note

破壊的変更として明記する。

- 以前はbreakpointがあると最初のbreakpointまで進んだ。
- 以後はLLDBのlaunch結果をそのまま返す。
- `STOP_AT_ENTRY`は利用者が指定する。

### 31. PR-05 test

C wrapperへ渡ったlaunch flagsをtestできるよう、`LaunchInfo#launch_flags`を確認する。

```ruby
info = LLDB::LaunchInfo.new
expect(info.launch_flags & STOP_AT_ENTRY).to eq(0)
```

targetにbreakpointを置いても`Target#launch`自身がcontinueを追加しないことを、event/stop IDまたはtest seamで確認する。

source上の禁止checkも追加できる。

```bash
! grep -R "wait_for_process_stop" lib/lldb
```

### 32. PR-05 commit案

1. `Remove implicit launch flags`
2. `Remove launch polling and auto-continue`
3. `Document direct launch semantics and migration`

### 33. PR-05完了条件

- `Target#launch`内にloop、sleep、Timeout、continueがない。
- 利用者未指定のflagを追加しない。
- `launch_with_info`を一度だけ呼ぶ。
- migration documentationがある。

---

## PR-06: FileSpec、dynamic buffer、sentinel

### 34. 目的

固定長path、曖昧な0、temporary string pointerを減らす。

### 35. 手順

#### 35.1 sentinelを一箇所へ集約する

最低限:

```text
INVALID_ADDRESS
INVALID_PROCESS_ID
INVALID_THREAD_ID
INVALID_BREAK_ID
INVALID_LINE_NUMBER
```

C++上流定数と一致することをcompile-timeまたはintegration testで確認する。

#### 35.2 `FileSpec`を追加する

C ABI:

```text
create/destroy/is_valid
get_filename
get_directory
get_path(buffer, length)
set_filename
set_directory
exists
```

Ruby:

```ruby
LLDB::FileSpec
#path
#filename
#directory
#exists?
```

pathはdynamic buffer helperを通す。

#### 35.3 path取得をFileSpecへ移す

対象:

- `Target#executable_path`
- `Frame#file_path`
- `Module#file_path`
- `Module#platform_file_path`

一度にpublic methodを削除せず、内部実装を`FileSpec#path`へ委譲する。

可能なら次も直接公開する。

```text
Target#executable_file
Frame#line_entry
Module#file
Module#platform_file
```

`LineEntry`が必要なら同じPRに詰め込みすぎず、FileSpecだけでpathを安全化してから別commitにする。

#### 35.4 stop descriptionをcaller bufferへ変更する

上流`SBThread::GetStopDescription`は`dst = nullptr`で必要長を返せるため、次の二段階にする。

1. null bufferで必要長を取得する。
2. 上限を検査してbufferを確保する。
3. 同じAPIを再度呼び、実際の返却長以内だけをcopyする。

二回の間にdescriptionが変わり必要長が増えた場合は、上限内で一度だけ再試行する。max size 0、上限超過、二回連続の長さ変化、truncationを明示的に扱う。

#### 35.5 address getterをauditする

対象例:

- `BreakpointLocation#load_address`
- `Value#load_address`
- `Watchpoint#watch_address`
- `Process#allocate_memory`

null/invalid wrapper時に0を返すC++ branchを上流sentinelへ合わせるか、statusでinvalid handleを返す。

### 36. PR-06 test

- 4096 byte超のpath
- multibyte/non-ASCII path
- empty path
- invalid FileSpec
- address 0が有効値の場合
- `INVALID_ADDRESS`
- max description length 0/1/exact/truncated
- repeated callで前のthread-local stringに依存しない

### 37. PR-06 commit案

1. `Expose LLDB sentinel constants`
2. `Add SBFileSpec bindings`
3. `Route path helpers through dynamic buffers`
4. `Bound stop descriptions`
5. `Preserve invalid address sentinels`

### 38. PR-06完了条件

- pathに固定4096 byte bufferを使わない。
- invalid addressを0と混同しない。
- synthesized C stringの寿命に依存しない。
- path helperは構造化FileSpecから得られる。

---

## PR-07: binding parity、RBS、package、sanitizer

### 39. 目的

新しいbinding追加時の層間欠落をCIで止める。

### 40. 手順

#### 40.1 `script/check_bindings`を作る

比較する集合:

```text
header declarations
shared library exported symbols
Ruby attach_function
RBS methods
```

最初はfunction名だけを比較する。型の完全比較は第二段階とする。

#### 40.2 surface ledgerを追加する

`bindings/surface.yml`で全low-level exportを次へ分類する。

```text
public: LLDB::Class#method
internal
metadata
optional_stub
deprecated
```

low-level functionとRuby methodは1対1とは限らない。checkerは名前からpublic methodを推測せず、未分類entry、存在しないpublic method、理由のないinternal分類を検出する。

#### 40.3 enum/sentinel parityを追加する

`bindings/constants.yml`へRuby constantと上流C++ enum/sentinelの対応を書く。選択されたLLDB headerで小さなprobeをcompile・実行し、Ruby値と比較する。

このledgerは定数検査専用であり、wrapper本体を生成しない。

#### 40.4 exception guard coverageを検査する

全exportが共通exception guardまたは明示的な`noexcept` metadata/error関数として分類されていることを検査する。

#### 40.5 RBS generationを再現可能にする

```bash
bundle exec rake rbs:generate
git diff --exit-code -- sig
```

生成結果が環境ごとに揺れる場合はtool versionをlockする。

#### 40.6 sanitizer job

CXXFLAGS/LDFLAGSへ次を追加するjobを作る。

```text
-fsanitize=address,undefined
-fno-omit-frame-pointer
```

同時指定が不安定ならASanとUBSanを分ける。

#### 40.7 package install layoutを固定する

PR-01のsmoke結果に基づき、custom Makefileのinstall先を修正する。推測で`sitelibdir`を使わず、RubyGemsが渡すinstall rootにwrapperが置かれることをartifact inspectionで確認する。

### 41. PR-07 test

checker自身のfixtureを作る。

- headerだけ追加した場合にfailure
- FFIだけ追加した場合にfailure
- RBS欠落でfailure
- low-level exportがsurface ledgerで未分類ならfailure
- public methodが存在しなければfailure
- Ruby enum/sentinelがC++ probeと異なればfailure
- exception guard未適用のexportがあればfailure
- 明示分類した差分だけsuccess

### 42. PR-07完了条件

- C++ symbolを追加してFFIを忘れるとCIが落ちる。
- FFIとRBSがずれるとCIが落ちる。
- low-level exportとRuby object surfaceの未分類差分でCIが落ちる。
- enum/sentinelの値が選択されたLLDB headerとずれるとCIが落ちる。
- C++ exception guardの欠落でCIが落ちる。
- built gem load testがsource treeへ依存しない。
- lifecycle/integration specがASan・UBSanで通る。

---

## PR-08: blocking callとGVL

### 43. 目的

LLDBのsynchronous call中に、無関係なRuby threadを止めない。

### 44. 手順

#### 44.1 先に測定testを作る

例:

1. 長く実行するfixture processを作る。
2. Ruby thread Aでcounterを増やす。
3. Ruby thread Bでblocking native callを呼ぶ。
4. call中にcounterが進むか確認する。

候補ごとに独立testする。

#### 44.2 `blocking: true`を限定適用する

候補:

- launch
- attach
- process continue
- thread step
- command interpreter
- listener wait

getter、short setter、destroyへ一括適用しない。

#### 44.3 callbackとの非互換を文書化する

現時点ではRuby callbackをnativeから呼ばない。将来callback APIを追加する場合、この設定の再検証を必須条件として設計コメントとreview checklistへ残す。

### 45. PR-08完了条件

- blocking call中に別Ruby threadが進む。
- native callの戻り値/errorが変わらない。
- race、double free、Context closeとの競合がTSAN相当のreviewで検討されている。
- `blocking: true`を理由なく全functionへ付けていない。

---

## PR-09: `SBListener` / `SBEvent` / `SBBroadcaster`

### 46. 目的

async modeをpollingなしで扱うためのPublic SB APIをbindする。

### 47. 変更対象

新規:

```text
lib/lldb/listener.rb
lib/lldb/event.rb
lib/lldb/broadcaster.rb
spec/lldb/listener_spec.rb
spec/lldb/event_spec.rb
spec/lldb/broadcaster_spec.rb
```

C header、C++、FFI、RBSも同じPRで追加する。

### 48. 第一段階のAPI

#### `Debugger`

```ruby
debugger.listener
debugger.broadcaster
```

#### `Process`

```ruby
process.broadcaster
```

#### `Listener`

```ruby
start_listening_for_events(broadcaster, mask)
stop_listening_for_events(broadcaster, mask)
wait_for_event(timeout_seconds:)
next_event
peek_event
```

`timeout_seconds:`は上流APIどおり0以上のIntegerとし、0はnon-blocking pollを意味する。`wait_for_event`はtimeout時`nil`を返す。errorとtimeoutを同一視しない。

#### `Event`

```ruby
valid?
type
broadcaster_class
data_flavor
description
process_event?
process_state
process
restarted?
interrupted?
```

### 49. ownership

- Listener/Event/Broadcasterも共通Handleを使う。
- `Debugger#listener`が返すcopyはDebugger Contextを保持する。
- `Event#process`は同じContextを持つ。
- Eventをqueueから取得した後も必要なSB object copyを所有する。

### 50. test

- timeout 0
- timeout後にeventなし
- launch state event
- stop event
- exit event
- broadcaster mask
- next/peekの違い
- Event close
- Debugger close後のEvent派生object

background threadや自動loopはtestにも実装しない。test側が明示loopを持つのはよい。

### 51. PR-09 commit案

1. `Bind SBBroadcaster`
2. `Bind SBEvent`
3. `Bind SBListener`
4. `Expose process event helpers`
5. `Document explicit event handling`

### 52. PR-09完了条件

- async modeをstate pollingなしで扱える。
- coreにbackground threadがない。
- timeoutとerrorを区別できる。
- eventから得たobjectのContextが正しい。

---

## PR-10: option object

### 53. 目的

bindingsがdefaultを決めず、利用者がPublic SB APIの選択肢を指定できるようにする。

### 54. 順序

#### 54.1 Debugger作成時の`source_init_files`

C ABIへ`source_init_files`を受け取るDebugger作成関数を追加し、Rubyでは次を公開する。

```ruby
LLDB::Debugger.create(source_init_files: false)
LLDB.create_debugger(source_init_files: false)
```

defaultは現行挙動を保つ`false`とする。`true`/`false`を上流`SBDebugger::Create(bool)`へそのまま渡し、bindings側でinit fileの場所や内容を選ばない。

integration testでは一時HOMEの`.lldbinit`に副作用のないsettingを置き、`false`で読まず`true`で読むことをCommandInterpreterから確認する。利用者の実HOMEをtestで参照しない。

#### 54.2 `LaunchInfo`の補完

最低限:

- arguments
- environment
- working directory
- launch flags
- listener
- executable file
- stdin/stdout/stderr pathまたはfile
- process plugin name
- shell

LLDB 14にないpropertyはcapabilityで守る。

#### 54.3 `AttachInfo`

- pid
- executable
- process name
- wait_for_launch
- ignore_existing
- listener
- plugin
- resume count

`Target#attach_with_info`を追加する。

#### 54.4 `ExpressionOptions`

- timeout
- unwind on error
- ignore breakpoints
- fetch dynamic value
- try all threads
- stop others
- language
- suppress persistent result

`Frame#evaluate_expression`と`Target#evaluate_expression`へoptional引数として渡す。

#### 54.5 `RunMode`

Ruby enumを追加し、step APIへ渡す。

### 55. test

各optionについて「Ruby getter/setterが動く」だけで終えず、少なくとも一つは実際のLLDB behaviorに反映されるintegration testを置く。

例:

- expression timeout
- ignore breakpoints
- launch environment
- attach wait flag
- run mode

### 56. 完了条件

- `source_init_files:`を上流`SBDebugger::Create(bool)`へそのまま渡す。
- native defaultをRuby側で上書きしない。
- option objectなしの既存callが従来の直接LLDB defaultと一致する。
- optional propertyはcapabilityで守られる。
- option objectはworkflowを実装しない。

---

## PR-11以降: API cluster completion

### 57. 原則

一つのclusterを完成させてから次へ進む。数だけ増やさない。

### 58. Cluster A: File / Address / Line

追加候補:

```text
SBAddress
SBLineEntry
SBFileSpecList
```

完了条件:

- Frame locationをStringへ潰さず取得できる。
- Breakpoint location addressをSBAddressとして扱える。
- file/load addressの区別を保持する。

### 59. Cluster B: Type

追加候補:

```text
SBTypeMember
field_at_index
direct_base_class_at_index
virtual_base_class_at_index
```

完了条件:

- `num_fields`だけでなく各fieldへ到達できる。
- offset、bitfield、type、nameを保持する。

### 60. Cluster C: Symbol

追加候補:

```text
SBSymbol
SBFunction
SBCompileUnit
SBBlock
```

完了条件:

- `Module#num_symbols`と`symbol_at_index`が対になる。
- function/symbolを名前Stringへ潰さない。

### 61. Cluster D: Instruction

追加候補:

```text
SBInstructionList
SBInstruction
```

完了条件:

- `Frame#disassemble`のString以外に構造化命令列を取得できる。
- address、mnemonic、operands、comment、bytesへ到達できる。

### 62. clusterごとのPR template

```markdown
## Upstream SB API

## Ownership

## Minimum LLDB / capability

## C ABI

## Ruby API

## RBS

## Integration fixture

## Out of scope
```

---

## 63. release順序

推奨:

### Bug-fix release

- PR-01
- PR-02
- PR-07のpackage/parity部分

### Binding ABI / error release

- PR-03
- PR-04（`Value#watch`修正を含む）
- PR-06

### Semantics-changing release

- PR-05
- PR-08

### Surface expansion release

- PR-09
- PR-10
- PR-11以降

実際のversion番号は現在のrelease policyに合わせる。ただし`Target#launch`変更を単なる内部修正としてreleaseしない。

## 64. 各PRの最終チェックリスト

```text
[x] bindingsの責務から外れるloop/retry/policyを追加していない
[x] LLDB 14でcompileする
[x] feature境界versionでcompileする
[x] 現行LLDBでcompileする
[x] C headerとC++ definitionが一致する
[x] exported symbolとFFIが一致する
[x] FFIとRBSが一致する
[x] low-level exportがpublic/internal等へ分類されている
[x] enum/sentinelが選択されたLLDB headerと一致する
[x] C++例外がC ABIを越えない
[x] ownershipが記述されている
[x] nullable/sentinel/errorがtestされている
[x] close/GCで二重releaseしない
[x] package install後にsource treeなしでrequireできる
[x] CHANGELOGに互換性影響がある
[x] README exampleが暗黙の旧launch挙動へ依存しない
[x] `source_init_files:`をbindings側で再解釈していない
[x] Linux/macOS以外を実装済みと表示していない
```

## 65. 実装中に判断を止める条件

次に当たった場合は、無理に同じPRへ入れない。

- Public SB APIではなくLLDB private APIが必要になる。
- optional APIをversion番号でしか判定できないように見える。
- objectのownershipを説明できない。
- errorをboolまたは0へ潰さないとRuby APIを保てない。
- event APIなしでpolling helperを入れたくなる。
- generated manifestが必要に見えるが、parity checkerで十分か未検証である。
- Windowsだけ別のABI設計が必要になる。
- LLDB 14対応のために現行LLDBの意味を変える必要がある。

その変更を独立した設計課題として切り出し、core bindingsへ妥協した挙動を入れない。
