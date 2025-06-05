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