# Project instructions

## `template-main.pdf` 始终保持为最新成品

- `out/template-main.pdf` 是本仓库受版本管理的正式编译产物，不是需要保留旧状态的用户源码。
- 修改任何会影响主模板的 `.tex`、代码片段或图片后，必须按仓库编译规范重新生成并直接覆盖 `out/template-main.pdf`，即使它在开始工作时已经是 dirty 状态。
- `out/template-main.pdf` 明确不适用 `no-dirty-write` 的“脏文件先快照、不得覆盖”限制；成功编译并验收后，应与本轮模板源码一起提交，确保仓库中的 PDF 永远对应最新工作树内容。
- 此例外仅适用于 `out/template-main.pdf`；`.tex`、代码、配置及其他用户改动仍按正常的脏文件保护和逐文件暂存规则处理。
