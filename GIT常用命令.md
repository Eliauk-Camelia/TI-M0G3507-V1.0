# Git 常用命令

## 分支操作
```bash
git branch                    # 查看本地分支
git branch -a                 # 查看所有分支（含远程）
git branch --show-current     # 只显示当前分支名
git switch main               # 切换到 main
git switch -c feature/xxx     # 新建并切换到 feature/xxx
git branch -d feature/xxx     # 删除本地分支
```

## 状态与历史
```bash
git status                    # 查看工作区状态
git log --oneline -10         # 最近 10 条提交（简洁）
git log --oneline --graph --all  # 所有分支可视化图
git diff                      # 查看未暂存的改动
git diff --stat               # 查看改动文件摘要
gitk --all &                  # GUI 历史浏览器
```

## 暂存与提交
```bash
git add <file>                # 暂存指定文件
git add .                     # 暂存所有改动
git commit -m "描述"          # 提交
git commit -am "描述"         # 暂存并提交所有已跟踪文件的改动
```

## 撤销与回滚
```bash
git restore <file>            # 撤销工作区改动
git restore --staged <file>   # 取消暂存
git reset --hard HEAD         # 丢弃所有本地改动，回到最新提交
git reset --hard <commit>     # 回到指定提交（危险）
```

## 远程操作
```bash
git push origin main          # 推送 main 到远程
git push origin feature/xxx   # 推送功能分支
git pull                      # 拉取并合并
git fetch                     # 只拉取，不合并
```

## 暂存工作区
```bash
git stash                     # 暂存当前改动
git stash pop                 # 恢复最近一次暂存
git stash list                # 查看暂存列表
```

## 清理
```bash
git clean -fd                 # 删除未跟踪的文件和目录（小心）
git clean -nd                 # 预览将被删除的文件（安全）
```
