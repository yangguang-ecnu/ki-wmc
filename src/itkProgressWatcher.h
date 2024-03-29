/*
 * itkProgressWatcher.h
 *
 *  Created on: Sep 5, 2012
 *      Author: soheil
 */

#ifndef ITKPROGRESSWATCHER_H_
#define ITKPROGRESSWATCHER_H_

#include "itkCommand.h"
#include "itkProcessObject.h"
#include "itkTimeProbe.h"
#include "util.h"
#include <iomanip>

namespace itk
{
  class ITKCommon_EXPORT ProgressWatcher
  {
  public:
    ProgressWatcher(itk::ProcessObject *o, const char *comment = "")
    {
      // Initialize state
      m_Process = o;
      m_Steps = 0;
      m_Comment = comment;
      m_TestAbort = false;
      m_Iterations = 0;
#if defined( _COMPILER_VERSION ) && ( _COMPILER_VERSION == 730 )
      m_Quiet = true;
#else
      m_Quiet = false;
#endif

      // Create a series of commands
      m_StartFilterCommand = CommandType::New();
      m_EndFilterCommand = CommandType::New();
      m_ProgressFilterCommand = CommandType::New();
      m_IterationFilterCommand = CommandType::New();
      m_AbortFilterCommand = CommandType::New();

      // Assign the callbacks
      m_StartFilterCommand->SetCallbackFunction(this,
          &ProgressWatcher::StartFilter);
      m_EndFilterCommand->SetCallbackFunction(this,
          &ProgressWatcher::EndFilter);
      m_ProgressFilterCommand->SetCallbackFunction(this,
          &ProgressWatcher::ShowProgress);
      m_IterationFilterCommand->SetCallbackFunction(this,
          &ProgressWatcher::ShowIteration);
      m_AbortFilterCommand->SetCallbackFunction(this,
          &ProgressWatcher::ShowAbort);

      // Add the commands as observers
      m_StartTag = m_Process->AddObserver(StartEvent(), m_StartFilterCommand);
      m_EndTag = m_Process->AddObserver(EndEvent(), m_EndFilterCommand);
      m_ProgressTag = m_Process->AddObserver(ProgressEvent(),
          m_ProgressFilterCommand);
      m_IterationTag = m_Process->AddObserver(IterationEvent(),
          m_IterationFilterCommand);
      m_AbortTag = m_Process->AddObserver(AbortEvent(), m_AbortFilterCommand);
    }

    /** Copy constructor */
    ProgressWatcher(const ProgressWatcher &watch)
    {
      // Remove any observers we have on the old process object
      if (m_Process)
        {
          if (m_StartFilterCommand)
            {
              m_Process->RemoveObserver(m_StartTag);
            }
          if (m_EndFilterCommand)
            {
              m_Process->RemoveObserver(m_EndTag);
            }
          if (m_ProgressFilterCommand)
            {
              m_Process->RemoveObserver(m_ProgressTag);
            }
          if (m_IterationFilterCommand)
            {
              m_Process->RemoveObserver(m_IterationTag);
            }
          if (m_AbortFilterCommand)
            {
              m_Process->RemoveObserver(m_AbortTag);
            }
        }

      // Initialize state
      m_TimeProbe = watch.m_TimeProbe;
      m_Process = watch.m_Process;
      m_Steps = watch.m_Steps;
      m_Iterations = watch.m_Iterations;
      m_Comment = watch.m_Comment;
      m_TestAbort = watch.m_TestAbort;
      m_Quiet = watch.m_Quiet;

      m_StartTag = 0;
      m_EndTag = 0;
      m_ProgressTag = 0;
      m_IterationTag = 0;
      m_AbortTag = 0;

      // Create a series of commands
      if (m_Process)
        {
          m_StartFilterCommand = CommandType::New();
          m_EndFilterCommand = CommandType::New();
          m_ProgressFilterCommand = CommandType::New();
          m_IterationFilterCommand = CommandType::New();
          m_AbortFilterCommand = CommandType::New();

          // Assign the callbacks
          m_StartFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::StartFilter);
          m_EndFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::EndFilter);
          m_ProgressFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowProgress);
          m_IterationFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowIteration);
          m_AbortFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowAbort);

          // Add the commands as observers
          m_StartTag = m_Process->AddObserver(StartEvent(),
              m_StartFilterCommand);
          m_EndTag = m_Process->AddObserver(EndEvent(), m_EndFilterCommand);
          m_ProgressTag = m_Process->AddObserver(ProgressEvent(),
              m_ProgressFilterCommand);
          m_IterationTag = m_Process->AddObserver(IterationEvent(),
              m_IterationFilterCommand);
          m_AbortTag = m_Process->AddObserver(AbortEvent(),
              m_AbortFilterCommand);
        }
    }

    /** Default constructor. Only provided so that you can have
     * std::vectors of ProgressWatchers. */
    ProgressWatcher()
    {
      // Initialize state
      m_Steps = 0;
      m_Comment = "Not watching an object";
      m_TestAbort = false;
      m_Iterations = 0;
#if defined( _COMPILER_VERSION ) && ( _COMPILER_VERSION == 730 )
      m_Quiet = true;
#else
      m_Quiet = false;
#endif

      m_Process = 0;
    }
    void
    operator=(const ProgressWatcher & watch)
    {
      // Remove any observers we have on the old process object
      if (m_Process)
        {
          if (m_StartFilterCommand)
            {
              m_Process->RemoveObserver(m_StartTag);
            }
          if (m_EndFilterCommand)
            {
              m_Process->RemoveObserver(m_EndTag);
            }
          if (m_ProgressFilterCommand)
            {
              m_Process->RemoveObserver(m_ProgressTag);
            }
          if (m_IterationFilterCommand)
            {
              m_Process->RemoveObserver(m_IterationTag);
            }
          if (m_AbortFilterCommand)
            {
              m_Process->RemoveObserver(m_AbortTag);
            }
        }

      // Initialize state
      m_TimeProbe = watch.m_TimeProbe;
      m_Process = watch.m_Process;
      m_Steps = watch.m_Steps;
      m_Iterations = watch.m_Iterations;
      m_Comment = watch.m_Comment;
      m_TestAbort = watch.m_TestAbort;
      m_Quiet = watch.m_Quiet;

      m_StartTag = 0;
      m_EndTag = 0;
      m_ProgressTag = 0;
      m_IterationTag = 0;
      m_AbortTag = 0;

      // Create a series of commands
      if (m_Process)
        {
          m_StartFilterCommand = CommandType::New();
          m_EndFilterCommand = CommandType::New();
          m_ProgressFilterCommand = CommandType::New();
          m_IterationFilterCommand = CommandType::New();
          m_AbortFilterCommand = CommandType::New();

          // Assign the callbacks
          m_StartFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::StartFilter);
          m_EndFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::EndFilter);
          m_ProgressFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowProgress);
          m_IterationFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowIteration);
          m_AbortFilterCommand->SetCallbackFunction(this,
              &ProgressWatcher::ShowAbort);

          // Add the commands as observers
          m_StartTag = m_Process->AddObserver(StartEvent(),
              m_StartFilterCommand);
          m_EndTag = m_Process->AddObserver(EndEvent(), m_EndFilterCommand);
          m_ProgressTag = m_Process->AddObserver(ProgressEvent(),
              m_ProgressFilterCommand);
          m_IterationTag = m_Process->AddObserver(IterationEvent(),
              m_IterationFilterCommand);
          m_AbortTag = m_Process->AddObserver(AbortEvent(),
              m_AbortFilterCommand);
        }
    }

    virtual
    ~ProgressWatcher()
    {
      // Remove any observers we have on the old process object
      if (m_Process)
        {
          if (m_StartFilterCommand)
            {
              m_Process->RemoveObserver(m_StartTag);
            }
          if (m_EndFilterCommand)
            {
              m_Process->RemoveObserver(m_EndTag);
            }
          if (m_ProgressFilterCommand)
            {
              m_Process->RemoveObserver(m_ProgressTag);
            }
          if (m_IterationFilterCommand)
            {
              m_Process->RemoveObserver(m_IterationTag);
            }
          if (m_AbortFilterCommand)
            {
              m_Process->RemoveObserver(m_AbortTag);
            }
        }
    }

    /** Method to get the name of the class be monitored by this
     *  ProgressWatcher */
    const char *
    GetNameOfClass()
    {
      return (m_Process.GetPointer() ? m_Process->GetNameOfClass() : "None");
    }

    /** Methods to control the verbosity of the messages. Quiet
     * reporting limits the information emitted at a ProgressEvent. */
    void
    QuietOn()
    {
      m_Quiet = true;
    }
    void
    QuietOff()
    {
      m_Quiet = false;
    }

    /** Methods to use to test the AbortEvent of the a filter. If
     * TestAbort is on, the filter being watched will be aborted when
     * the progress reaches 30%. */
    void
    TestAbortOn()
    {
      m_TestAbort = true;
    }
    void
    TestAbortOff()
    {
      m_TestAbort = false;
    }

    /** Methods to access member data */
    /** Get a pointer to the process object being watched. */
    ProcessObject *
    GetProcess()
    {
      return m_Process.GetPointer();
    }

    /** Set/Get the steps completed. */
    void
    SetSteps(int val)
    {
      m_Steps = val;
    }
    int
    GetSteps()
    {
      return m_Steps;
    }

    /** Set/Get the number of iterations completed. */
    void
    SetIterations(int val)
    {
      m_Iterations = val;
    }
    int
    GetIterations()
    {
      return m_Iterations;
    }

    /** Set/Get the quiet mode boolean. If true, verbose progress is
     * reported. */
    void
    SetQuiet(bool val)
    {
      m_Quiet = val;
    }
    bool
    GetQuiet()
    {
      return m_Quiet;
    }

    /** Get the comment for the watcher. */
    std::string
    GetComment()
    {
      return m_Comment;
    }

    /** Get a reference to the TimeProbe */
    TimeProbe &
    GetTimeProbe()
    {
      return m_TimeProbe;
    }

  protected:

    /** Callback method to show the ProgressEvent */
    virtual void
    ShowProgress()
    {
      if (m_Process)
        {
          m_Steps++;
          if (!m_Quiet)
            {
              printProgress(m_Process->GetProgress());
            }
          if (m_TestAbort)
            {
              if (m_Process->GetProgress() > .03)
                {
                  m_Process->AbortGenerateDataOn();
                }
            }
        }
    }

    /** Callback method to show the AbortEvent */
    virtual void
    ShowAbort()
    {
      std::cout << std::endl << "-------Aborted" << std::endl << std::flush;
    }

    /** Callback method to show the IterationEvent */
    virtual void
    ShowIteration()
    {
      std::cout << " #" << std::flush;
      m_Iterations++;
    }

    /** Callback method to show the StartEvent */
    virtual void
    StartFilter()
    {
      m_Steps = 0;
      m_Iterations = 0;
      m_TimeProbe.Start();
      std::cout << "Start "
          << (m_Process.GetPointer() ? m_Process->GetNameOfClass() : "None")
          << " \"" << m_Comment << "\" " << std::endl;
    }

    /** Callback method to show the EndEvent */
    virtual void
    EndFilter()
    {
      m_TimeProbe.Stop();
      std::cout << std::endl << "Filter took " << m_TimeProbe.GetMeanTime()
          << " seconds.";
      std::cout << std::endl << "End "
          << (m_Process.GetPointer() ? m_Process->GetNameOfClass() : "None")
          << " \"" << m_Comment << "\" " << std::endl;
      if (m_Steps < 1)
        {
          itkExceptionMacro("Filter does not have progress.");
        }
    }

  private:

    TimeProbe m_TimeProbe;
    int m_Steps;
    int m_Iterations;
    bool m_Quiet;
    bool m_TestAbort;
    std::string m_Comment;
    itk::ProcessObject::Pointer m_Process;

    typedef SimpleMemberCommand<ProgressWatcher> CommandType;
    CommandType::Pointer m_StartFilterCommand;
    CommandType::Pointer m_EndFilterCommand;
    CommandType::Pointer m_ProgressFilterCommand;
    CommandType::Pointer m_IterationFilterCommand;
    CommandType::Pointer m_AbortFilterCommand;

    unsigned long m_StartTag;
    unsigned long m_EndTag;
    unsigned long m_ProgressTag;
    unsigned long m_IterationTag;
    unsigned long m_AbortTag;
  };
} // end namespace itk

#endif /* ITKPROGRESSWATCHER_H_ */
