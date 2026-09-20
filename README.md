# media

孤儿分支，只放 PR / issue 里引用的图片，不参与主分支历史。

- 目录：`pr/<主题>/<文件名>.png`
- 引用：`https://raw.githubusercontent.com/znzryb/icpc-ccpc-template/media/pr/<主题>/<文件名>.png`
- 往这里加图：`git checkout media && cp ... pr/<主题>/ && git add . && git commit && git push origin media`
