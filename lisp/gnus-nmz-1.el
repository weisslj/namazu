;;; gnus-nmz-1.el --- interface between Namazu and Gnus.

;; Author: KOSEKI Yoshinori <kose@yk.NetLaputa.ne.jp>
;; Keywords: mail, news

;;; Code:

(defconst gnus-nmz-version "gnus-nmz -1.0.1"
  "Version string for this version of gnus-nmz-1.")

(require 'namazu)

(defvar gnus-nmz-nnml-spool-regex 
  (concat (expand-file-name "~/") "Mail/\\(.+\\)/")
  "*検索結果中の Gnus の Mail spool のパターン")

(defvar gnus-nmz-cache-regex 
    (concat (expand-file-name "~/") "News/cache/\\([^/]+\\)/\\([0-9]+\\)")
    "*検索結果中の Gnus の News cache のパターン")

(if (boundp 'namazu-view-function-alist)
    (setq namazu-view-function-alist
          (cons (cons (concat gnus-nmz-nnml-spool-regex
			      "\\|"
			      gnus-nmz-cache-regex)
		      'gnus-nmz-view)
		namazu-view-function-alist)))

(defvar gnus-nmz-with-windows  (featurep 'windows)
  "*nil 以外の値を設定すると、windows.el
(http://www.gentei.org/~yuuji/software/)
を使います。もちろん別途インストールの必要あり。")
  
(defun gnus-nmz-view (path)
  (interactive)
  (let (group id)
    (cond 
     ((string-match gnus-nmz-nnml-spool-regex path)
      (setq group (match-string 1 path))
      ;; replace "/" with "."
      (let (result (start 0))
	(while (string-match "/" group start)
	  (setq result (concat result
			       (substring group start (match-beginning 0))
			       "."))
	  (setq start (match-end 0)))
	(setq group (concat result (substring group start))))
      (setq group (format "nnml:%s" group))
      (setq id (format "%s" (file-name-nondirectory path))))
     ((string-match gnus-nmz-cache-regex path)
      (setq group (format "%s"(substring path
                                         (match-beginning 1) (match-end 1))))
      (setq id (format "%s" (substring path
                                       (match-beginning 2) (match-end 2)))))
     (t ))
    (if group
        (save-excursion
          (if gnus-nmz-with-windows
              (win-switch-to-window 0 2))
          (if (get-buffer gnus-group-buffer)
              (set-buffer gnus-group-buffer)
            (gnus)
            (set-buffer gnus-group-buffer))
          (if gnus-topic-mode
              (gnus-topic-read-group 1 nil group)
            (gnus-group-read-group 1 nil group))
          (gnus-summary-goto-article id nil t)
          (sit-for 0)
          (gnus-summary-refer-thread)))))

(eval-after-load "gnus-sum"
  '(define-key gnus-summary-mode-map "q" 'gnus-nmz-gnus-summary-exit))

(defun gnus-nmz-gnus-summary-exit ()
  (interactive)
  (if (null (get-buffer namazu-buffer))
      (gnus-summary-exit)
    (delete-other-windows)
    (switch-to-buffer namazu-buffer)))

(provide 'gnus-nmz-1)

;;; gnus-nmz-1.el ends here

;;; Local Variables: ;;;
;;; change-log-default-name: "ChangeLog.gnus-nmz-1" ;;;
;;; End: ;;;
