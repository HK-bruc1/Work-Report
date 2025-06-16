# 拉取主分支会一起更新所有分支吗？

不会，使用 Git 拉取远端主分支（如 `git pull origin main`）时，**只会更新当前所在分支（如本地的 `main` 分支）及其对应的远程分支（如 `origin/main`）**，**不会自动更新所有本地分支或远程分支的引用**。

## `git pull origin main` 会做什么？

该命令相当于：

```bash
git fetch origin main
git merge origin/main
```

也就是说，它会：

- 从远程仓库 `origin` 拉取 `main` 分支的最新提交；
- 将远程的 `origin/main` 合并到当前所在的本地分支。

## 不会更新哪些分支？

- 其他本地分支（如 `dev`, `feature/test` 等）不会被更新；
- 其他远程分支（如 `origin/dev`, `origin/feature/test` 等）也不会更新。

## 如果想更新所有远程分支引用怎么办？

```bash
git fetch --all
```

该命令会从远程仓库拉取**所有分支的最新引用信息**，但不会合并到本地分支，只是更新 `origin/*` 的引用。

执行完后，远程分支的最新状态就都同步到了本地的 `origin/*` 分支，例如：

- `origin/main`
- `origin/dev`
- `origin/feature/xxx`

但你本地的实际分支（如 `main`, `dev` 等）不会被修改。

# 撤销本地提交

我本地领先远程两个提交，现在这两个领先的提交我想丢弃掉

## 先fetch再reset

为了确保远程信息是最新的：

```bash
# 获取最新的远程信息
git fetch origin

# 重置到远程分支
git reset --hard origin/main
```

### 验证结果

执行后可以检查状态：

```bash
# 查看当前状态
git status

# 查看提交历史
git log --oneline -5
```

你应该看到类似这样的输出：

```bash
Your branch is up to date with 'origin/main'.
nothing to commit, working tree clean
```

### 注意事项

- `--hard` 参数会**完全删除**这两个提交的所有更改，包括工作区的修改
- 如果你想保留这些更改到工作区，可以使用 `--mixed`（默认）或 `--soft`
- 执行前确保没有重要的未保存工作
- 如果需要，可以先创建一个备份分支：`git branch backup-branch`

这样操作后，你的本地分支就会与远程分支完全同步，那两个领先的提交会被丢弃。

## 使用undo commit

可以把最新的本地提交撤销，所有更改都暂存在工作区中，提交信息也在输入框。