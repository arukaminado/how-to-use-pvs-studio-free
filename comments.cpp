//  2006-2008 (c) Viva64.com Team
//  2008-2016 (c) OOO "Program Verification Systems"

#include "comments.h"
#include <cassert>
#include <cctype>

const std::vector<PvsStudioFreeComments::Comment> PvsStudioFreeComments::Comments = {
#include "comments_msg.h"
};

static int strcasecmp(const std::string &lhs, const std::string &rhs)
{
  if (lhs.length() < rhs.length())
  {
    return -1;
  }
  else if (lhs.length() > rhs.length())
  {
    return 1;
  }

  for (size_t i = 0; i < lhs.length(); ++i)
  {
    int d = tolower(lhs[i]) - tolower(rhs[i]);
    if (d != 0)
    {
      return d;
    }
  }
  return 0;
}

class CommentsParser
{
public:
  bool readFreeComment(const char* buf)
  {
    m_comment.trimmedText.clear();
    m_comment.lines = 0;
    m_skippedLines = 0;
    m_inMultiline = false;
    size_t prevLines = 0;
    while (buf != nullptr && *buf != '\0')
    {
      buf = readComment(buf);
      if (m_comment.lines != prevLines)
      {
        bool needMoreLines = false;
        for (auto &comment : PvsStudioFreeComments::Comments)
        {
          if (comment.lines == m_comment.lines)
          {
            if (strcasecmp(comment.trimmedText, m_comment.trimmedText) == 0)
              return true;
          }
          else if (comment.lines >= m_comment.lines)
            needMoreLines = true;
        }
        prevLines = m_comment.lines;
        if (!needMoreLines)
          break;
      }
      else
      {
        ++m_skippedLines;
      }
    }
    return false;
  }

private:
  const char* readComment(const char* it)
  {
    while (isspace(*it) && m_skippedLines < MaxSkippedLines)
    {
      if (it[0] == '\n')
        ++m_skippedLines;
      ++it;
    }

    if (m_skippedLines >= MaxSkippedLines)
      return nullptr;

    if (m_inMultiline)
      return readMultilineComment(it);

    if (it[0] == '/' && it[1] == '/')
    {
      const char *begin = it + 2;
      const char *end = begin;
      while (*end != '\n' && *end != '\0')
        ++end;
      it = (*end == '\n') ? end + 1 : end;

      m_comment.addLine(begin, end);
      return it;
    }
    if (it[0] == '/' && it[1] == '*')
    {
      m_inMultiline = true;
      return readMultilineComment(it + 2);
    }

    return nullptr;
  }

  const char* readMultilineComment(const char* it)
  {
    if (it[0] == '*' && it[1] != '/')
      ++it;

    const char *begin = it;
    const char *end = begin;
    while (*end != '\n' && *end != '\0' && (end[0] != '*' || end[1] != '/'))
      ++end;

    switch (*end)
    {
    case '\n':
      it = end + 1;
      break;
    case '\0':
      it = end;
      break;
    case '*':
      it = end + 2;
      m_inMultiline = false;
      break;
    default:
      assert(false);
    }

    m_comment.addLine(begin, end);
    return it;
  }

  static constexpr size_t MaxSkippedLines = 10;

  PvsStudioFreeComments::Comment m_comment;
  size_t m_skippedLines;
  bool m_inMultiline;
};


bool PvsStudioFreeComments::HasAnyComment(const char* buf)
{
  CommentsParser parser;
  return parser.readFreeComment(buf);
}
